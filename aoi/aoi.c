#include "aoi.h"
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define AOI_RADIS 10.0f

#define INVALID_ID (~0)
#define PRE_ALLOC 16
#define AOI_RADIS2 (AOI_RADIS * AOI_RADIS)
#define DIST2(p1, p2)                                                          \
	((p1[0] - p2[0]) * (p1[0] - p2[0]) + (p1[1] - p2[1]) * (p1[1] - p2[1]) +   \
	 (p1[2] - p2[2]) * (p1[2] - p2[2]))
#define MODE_WATCHER 1
#define MODE_MARKER 2
#define MODE_MOVE 4
#define MODE_DROP 8

typedef struct _object {
	int ref;
	uint32_t id;
	int version;
	int mode;
	float last[3];
	float position[3];
} object;

typedef struct _object_set {
	int cap;
	int number;
	object **slot;
} object_set;

typedef struct _pair_list {
	struct _pair_list *next;
	object *watcher;
	object *marker;
	int watcher_version;
	int marker_version;
} pair_list;

typedef struct _map_slot {
	uint32_t id;
	object *obj;
	int next;
} map_slot;

typedef struct _map {
	int size;
	int lastfree;
	map_slot *slot;
} map;

struct aoi_space {
	aoi_Alloc alloc;
	void *alloc_ud;
	map *object;
	object_set *wathcer_static;
	object_set *marker_static;
	object_set *wathcer_move;
	object_set *marker_move;
	pair_list *hot;
};

static object *
new_object(struct aoi_space *space, uint32_t id) {
	object *obj = space->alloc(space->alloc_ud, NULL, sizeof(*obj));
	obj->ref = 1;
	obj->id = id;
	obj->version = 0;
	obj->mode = 0;
	return obj;
}

static inline map_slot *
mainpostion(map *m, uint32_t id) {
	uint32_t hash = id & (m->size - 1);
	return &m->slot[hash];
}

static void rehash(struct aoi_space *space, map *m);

static void
map_insert(struct aoi_space *space, map *m, uint32_t id, object *obj) {
	map_slot *s = mainpostion(m, id);
	if (s->id == INVALID_ID) {
		s->id = id;
		s->obj = obj;
		return;
	}

	if (mainpostion(m, s->id) != s) {
		map_slot *last = mainpostion(m, s->id);
		while (last->next != s - m->slot) {
			assert(last->next >= 0);
			last = &m->slot[last->next];
		}
		uint32_t temp_id = s->id;
		object *temp_obj = s->obj;
		last->next = s->next;
		s->id = id;
		s->obj = obj;
		s->next = -1;
		if (temp_obj) {
			map_insert(space, m, temp_id, temp_obj);
		}
		return;
	}

	while (m->lastfree >= 0) {
		map_slot *temp = &m->slot[m->lastfree--];
		if (temp->id == INVALID_ID) {
			temp->id = id;
			temp->obj = obj;
			temp->next = s->next;
			s->next = (int)(temp - m->slot);
			return;
		}
	}

	rehash(space, m);
	map_insert(space, m, id, obj);
}

static void
rehash(struct aoi_space *space, map *m) {
	map_slot *old_slot = m->slot;
	int old_size = m->size;
	m->size = 2 * old_size;
	m->lastfree = m->size - 1;
	m->slot = space->alloc(space->alloc_ud, NULL, m->size * sizeof(map_slot));
	int i;
	for (i = 0; i < m->size; i++) {
		map_slot *s = &m->slot[i];
		s->id = INVALID_ID;
		s->obj = NULL;
		s->next = -1;
	}
	for (i = 0; i < old_size; i++) {
		map_slot *s = &old_slot[i];
		if (s->obj)
			map_insert(space, m, s->id, s->obj);
	}
	space->alloc(space->alloc_ud, old_slot, old_size * sizeof(map_slot));
}

static object *
map_query(struct aoi_space *space, map *m, uint32_t id) {
	map_slot *s = mainpostion(m, id);
	for (;;) {
		if (s->id == id) {
			if (s->obj == NULL) {
				s->obj = new_object(space, id);
			}
			return s->obj;
		}
		if (s->next < 0)
			break;
		s = &m->slot[s->next];
	}

	object *obj = new_object(space, id);
	map_insert(space, m, id, obj);
	return obj;
}

static void
map_foreach(map *m, void (*func)(void *ud, object *obj), void *ud) {
	int i;
	for (i = 0; i < m->size; i++) {
		if (m->slot[i].obj)
			func(ud, m->slot[i].obj);
	}
}

static object *
map_drop(map *m, uint32_t id) {
	uint32_t hash = id & (m->size - 1);
	map_slot *s = &m->slot[hash];
	for (;;) {
		if (s->id == id) {
			object *obj = s->obj;
			s->obj = NULL;
			return obj;
		}
		if (s->next < 0)
			return NULL;
		s = &m->slot[s->next];
	}
}

static void
map_delete(struct aoi_space *space, map *m) {
	space->alloc(space->alloc_ud, m->slot, m->size * sizeof(map_slot));
	space->alloc(space->alloc_ud, m, sizeof(*m));
}

static map *
map_new(struct aoi_space *space) {
	int i;
	map *m = space->alloc(space->alloc_ud, NULL, sizeof(*m));
	m->size = PRE_ALLOC;
	m->lastfree = PRE_ALLOC - 1;
	m->slot = space->alloc(space->alloc_ud, NULL, m->size * sizeof(map_slot));
	for (i = 0; i < m->size; i++) {
		map_slot *s = &m->slot[i];
		s->id = INVALID_ID;
		s->obj = NULL;
		s->next = -1;
	}

	return m;
}

inline static void
grab_object(object *obj) {
	++obj->ref;
}

static void
delete_object(void *s, object *obj) {
	struct aoi_space *space = s;
	space->alloc(space->alloc_ud, obj, sizeof(*obj));
}

inline static void
drop_object(struct aoi_space *space, object *obj) {
	--obj->ref;
	if (obj->ref <= 0) {
		map_drop(space->object, obj->id);
		delete_object(space, obj);
	}
}

static object_set *
set_new(struct aoi_space *space) {
	object_set *set = space->alloc(space->alloc_ud, NULL, sizeof(*set));
	set->cap = PRE_ALLOC;
	set->number = 0;
	set->slot =
		space->alloc(space->alloc_ud, NULL, set->cap * sizeof(object *));

	return set;
}

struct aoi_space *
aoi_create(aoi_Alloc alloc, void *ud) {
	struct aoi_space *space = alloc(ud, NULL, sizeof(*space));
	space->alloc = alloc;
	space->alloc_ud = ud;
	space->object = map_new(space);
	space->wathcer_static = set_new(space);
	space->marker_static = set_new(space);
	space->wathcer_move = set_new(space);
	space->marker_move = set_new(space);
	space->hot = NULL;

	return space;
}

static void
delete_pair_list(struct aoi_space *space) {
	pair_list *p = space->hot;
	while (p) {
		pair_list *next = p->next;
		space->alloc(space->alloc_ud, p, sizeof(*p));
		p = next;
	}
}

static void
delete_set(struct aoi_space *space, object_set *set) {
	if (set->slot) {
		space->alloc(space->alloc_ud, set->slot, sizeof(object *) * set->cap);
	}
	space->alloc(space, set, sizeof(*set));
}

void
aoi_release(struct aoi_space *space) {
	map_foreach(space->object, delete_object, space);
	map_delete(space, space->object);
	delete_pair_list(space);
	delete_set(space, space->wathcer_static);
	delete_set(space, space->marker_static);
	delete_set(space, space->wathcer_move);
	delete_set(space, space->marker_move);
	space->alloc(space->alloc_ud, space, sizeof(space));
}

inline static void
copy_position(float des[3], float src[3]) {
	des[0] = src[0];
	des[1] = src[1];
	des[2] = src[2];
}

static bool
change_mode(object *obj, bool set_watcher, bool set_marker) {
	bool change = false;
	if (obj->mode == 0) {
		if (set_watcher)
			obj->mode = MODE_WATCHER;
		if (set_marker)
			obj->mode |= MODE_MARKER;

		return true;
	}

	if (set_watcher) {
		if (!(obj->mode & MODE_WATCHER)) {
			obj->mode |= MODE_WATCHER;
			change = true;
		}
	} else {
		if (obj->mode & MODE_WATCHER) {
			obj->mode &= ~MODE_WATCHER;
			change = true;
		}
	}

	if (set_marker) {
		if (!(obj->mode & MODE_MARKER)) {
			obj->mode |= MODE_MARKER;
			change = true;
		}
	} else {
		if (obj->mode & MODE_MARKER) {
			obj->mode &= ~MODE_MARKER;
			change = true;
		}
	}

	return change;
}

inline static bool
is_near(float p1[3], float p2[3]) {
	return DIST2(p1, p2) < AOI_RADIS2 * 0.25f;
}

inline static float
dist2(object *p1, object *p2) {
	float d = DIST2(p1->position, p2->position);
	return d;
}

void
aoi_update(struct aoi_space *space, uint32_t id, const char *modestring,
		   float pos[3]) {
	object *obj = map_query(space, space->object, id);
	int i;
	bool set_watcher = false;
	bool set_marker = false;

	for (i = 0; modestring[i]; i++) {
		char m = modestring[i];
		switch (m) {
		case 'w':
			set_watcher = true;
			break;
		case 'm':
			set_marker = true;
			break;
		case 'd':
			if (!(obj->mode & MODE_DROP)) {
				obj->mode = MODE_DROP;
				drop_object(space, obj);
			}
			return;
		}
	}

	if (obj->mode & MODE_DROP) {
		obj->mode &= ~MODE_DROP;
		grab_object(obj);
	}

	bool changed = change_mode(obj, set_watcher, set_marker);

	copy_position(obj->position, pos);
	if (changed || !is_near(pos, obj->last)) {
		copy_position(obj->last, pos);
		obj->mode |= MODE_MOVE;
		++obj->version;
	}
}

static void
drop_pair(struct aoi_space *space, pair_list *p) {
	drop_object(space, p->watcher);
	drop_object(space, p->marker);
	space->alloc(space->alloc_ud, p, sizeof(*p));
}

static void
flush_pair(struct aoi_space *space, aoi_Callback cb, void *ud) {
	pair_list **last = &(space->hot);
	pair_list *p = *last;
	while (p) {
		pair_list *next = p->next;
		if (p->watcher->version != p->watcher_version ||
			p->marker->version != p->marker_version ||
			(p->watcher->mode & MODE_DROP) || (p->marker->mode & MODE_DROP)) {

			drop_pair(space, p);
			*last = next;
		} else {
			float distance2 = dist2(p->watcher, p->marker);
			if (distance2 > AOI_RADIS2 * 4) {
				drop_pair(space, p);
				*last = next;
			} else if (distance2 < AOI_RADIS2) {
				cb(ud, p->watcher->id, p->marker->id);
				drop_pair(space, p);
				*last = next;
			} else {
				last = &(p->next);
			}
		}
		p = next;
	}
}

static void
set_push_back(struct aoi_space *space, object_set *set, object *obj) {
	if (set->number >= set->cap) {
		int cap = set->cap * 2;
		void *tmp = set->slot;
		set->slot = space->alloc(space->alloc_ud, NULL, cap * sizeof(object *));
		memcpy(set->slot, tmp, set->cap * sizeof(object *));
		space->alloc(space->alloc_ud, tmp, set->cap * sizeof(object *));
		set->cap = cap;
	}
	set->slot[set->number] = obj;
	++set->number;
}

static void
set_push(void *s, object *obj) {
	struct aoi_space *space = s;
	int mode = obj->mode;
	if (mode & MODE_WATCHER) {
		if (mode & MODE_MOVE) {
			set_push_back(space, space->wathcer_move, obj);
			obj->mode &= ~MODE_MOVE;
		} else {
			set_push_back(space, space->wathcer_static, obj);
		}
	}

	if (mode & MODE_MARKER) {
		if (mode & MODE_MARKER) {
			set_push_back(space, space->marker_move, obj);
			obj->mode &= ~MODE_MOVE;
		} else {
			set_push_back(space, space->marker_static, obj);
		}
	}
}

static void
gen_pair(struct aoi_space *space, object *watchcer, object *marker,
		 aoi_Callback cb, void *ud) {
	if (watchcer == marker) {
		return;
	}
	float distance2 = dist2(watchcer, marker);
	if (distance2 < AOI_RADIS2) {
		cb(ud, watchcer->id, marker->id);
		return;
	}
	pair_list *p = space->alloc(space->alloc_ud, NULL, sizeof(*p));
	p->watcher = watchcer;
	grab_object(watchcer);
	p->marker = marker;
	grab_object(marker);
	p->watcher_version = watchcer->version;
	p->marker_version = marker->version;
	p->next = space->hot;
	space->hot = p;
}

static void
gen_pair_list(struct aoi_space *space, object_set *watcher, object_set *marker,
			  aoi_Callback cb, void *ud) {
	int i, j;
	for (i = 0; i < watcher->number; i++) {
		for (j = 0; j < marker->number; j++) {
			gen_pair(space, watcher->slot[i], marker->slot[j], cb, ud);
		}
	}
}

void
aoi_message(struct aoi_space *space, aoi_Callback cb, void *ud) {
	flush_pair(space, cb, ud);
	space->wathcer_static->number = 0;
	space->wathcer_move->number = 0;
	space->marker_static->number = 0;
	space->marker_move->number = 0;
	map_foreach(space->object, set_push, space);
	gen_pair_list(space, space->wathcer_static, space->marker_move, cb, ud);
	gen_pair_list(space, space->wathcer_move, space->marker_static, cb, ud);
	gen_pair_list(space, space->wathcer_move, space->marker_move, cb, ud);
}

static void *
default_alloc(void *ud, void *ptr, size_t sz) {
	if (ptr == NULL) {
		void *p = malloc(sz);
		return p;
	}
	free(ptr);
	return NULL;
}

struct aoi_space *
aoi_new() {
	return aoi_create(default_alloc, NULL);
}
#ifndef __SPINLOCK_H
#define __SPINLOCK_H

#define SPIN_INIT(q) spinlock_init(&(q)->lock);
#define SPIN_LOCK(q) spinlock_lock(&(q)->lock);
#define SPIN_UNLOCK(q) spinlock_unlock(&(q)->lock);
#define SPIN_DESTROY(q) spinlock_destroy(&(q)->lock);

struct spinlock{
  int lock;
};

static inline void
spinlock_init(struct spinlock *lock){
  lock->lock = 0;
}

static inline void
spinlock_lock(struct spinlock *lock){
  while(__sync_lock_test_and_set(&lock->lock,1)){};
}

static inline int
spinlock_trylock(struct spinlock *lock){
  return __sync_lock_test_and_set(&lock->lock,1) == 1;
}

static inline void
spinlock_unlock(struct spinlock *lock){
  __sync_lock_release(&lock->lock);
}

static inline void
spinlock_destroy(struct spinlock *lock){
  (void)lock;
}
#endif

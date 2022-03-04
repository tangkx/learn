#ifndef __MACRO_H
#define __MACRO_H

#define t_offsetof(type, member) ((size_t) &((type *)0)->member)

#define t_container_of(ptr, type, member) ({                \
    const typeof( ((type *)0)->member ) *__mptr = (ptr);    \
    (type *) ( (char *)__mptr - t_offsetof(type,member) ); })

#endif
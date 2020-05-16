#ifndef __LEONBARE_LIBLOCKS_H
#define __LEONBARE_LIBLOCKS_H

extern int (*__lbst_pthread_mutex_init)        (pthread_mutex_t *__mutex, pthread_mutexattr_t *__mutex_attr); 
extern int (*__lbst_pthread_mutex_destroy)     (pthread_mutex_t *__mutex); 
extern int (*__lbst_pthread_mutex_trylock)     (pthread_mutex_t *__mutex); 
extern int (*__lbst_pthread_mutex_lock)        (pthread_mutex_t *__mutex); 
extern int (*__lbst_pthread_mutex_unlock)      (pthread_mutex_t *__mutex); 
extern int (*__lbst_pthread_mutexattr_init)    (pthread_mutexattr_t *__attr); 
extern int (*__lbst_pthread_mutexattr_destroy) (pthread_mutexattr_t *__attr); 
extern int (*__lbst_pthread_mutexattr_settype) (pthread_mutexattr_t *__attr, int __kind); 

#endif /* __LEONBARE_LIBLOCKS_H */

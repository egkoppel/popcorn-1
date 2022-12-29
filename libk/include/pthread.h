
#ifndef POPCORN_LIBK_INCLUDE_PTHREAD_H
#define POPCORN_LIBK_INCLUDE_PTHREAD_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

	typedef void *pthread_key_t;
	typedef void *pthread_once_t;
	typedef void *pthread_mutex_t;
	typedef void *pthread_mutexattr_t;
	typedef void *pthread_cond_t;

#define PTHREAD_MUTEX_INITIALIZER NULL
#define PTHREAD_COND_INITIALIZER  NULL
#define PTHREAD_ONCE_INIT         NULL

	int sched_yield();
	int pthread_key_create(pthread_key_t *key, void (*)(void *));
	int pthread_once(pthread_once_t *control, void (*init)(void));
	void *pthread_getspecific(pthread_key_t key);
	int pthread_setspecific(pthread_key_t key, const void *data);
	int pthread_mutex_init(pthread_mutex_t *mutex, const pthread_mutexattr_t *);
	int pthread_mutex_lock(pthread_mutex_t *mutex);
	int pthread_mutex_unlock(pthread_mutex_t *mutex);
	int pthread_cond_wait(pthread_cond_t *, pthread_mutex_t *);
	int pthread_cond_signal(pthread_cond_t *);

#ifdef __cplusplus
}
#endif

#endif   //POPCORN_LIBK_INCLUDE_PTHREAD_H


#ifndef _SYS_SEMAPHORE_H
#define _SYS_SEMAPHORE_H

#ifdef __cplusplus
extern "C" {
#endif

typedef int sem_t;

sem_t *sem_open(const char *, int, ...);
int    sem_close(sem_t *);

int    sem_init(sem_t *, int, unsigned);
int    sem_destroy(sem_t *);

int    sem_getvalue(sem_t *, int *);
int    sem_post(sem_t *);

int    sem_timedwait(sem_t *, const struct timespec *);
int    sem_trywait(sem_t *);
int    sem_wait(sem_t *);

int    sem_unlink(const char *);

#ifdef __cplusplus
}
#endif

#endif


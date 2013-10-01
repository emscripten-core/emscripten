#ifndef _SYS_SHM_H
#define _SYS_SHM_H

#ifdef __cplusplus
extern "C" {
#endif

#include <features.h>

#define __NEED_time_t
#define __NEED_size_t
#define __NEED_pid_t

#include <bits/alltypes.h>

#include <sys/ipc.h>
#include <bits/shm.h>

#define SHM_R 0400
#define SHM_W 0200

#define SHM_RDONLY 010000
#define SHM_RND    020000
#define SHM_REMAP  040000
#define SHM_EXEC   0100000

#define SHM_LOCK 11
#define SHM_UNLOCK 12
#define SHM_STAT 13
#define SHM_INFO 14
#define SHM_DEST 01000
#define SHM_LOCKED 02000
#define SHM_HUGETLB 04000
#define SHM_NORESERVE 010000

struct shminfo {
	unsigned long shmmax, shmmin, shmmni, shmseg, shmall, __unused[4];
};

struct shm_info {
	int used_ids;
	unsigned long shm_tot, shm_rss, shm_swp;
#ifdef _GNU_SOURCE
	unsigned long swap_attempts, swap_successes;
#else
	unsigned long __reserved[2];
#endif
};

typedef unsigned long shmatt_t;

void *shmat(int, const void *, int);
int shmctl(int, int, struct shmid_ds *);
int shmdt(const void *);
int shmget(key_t, size_t, int);

#ifdef __cplusplus
}
#endif

#endif

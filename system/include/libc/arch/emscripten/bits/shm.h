#define SHMLBA 4096

struct shmid_ds
{
	struct ipc_perm shm_perm;
	size_t shm_segsz;
	time_t shm_atime;
	int __unused1;
	time_t shm_dtime;
	int __unused2;
	time_t shm_ctime;
	int __unused3;
	pid_t shm_cpid;
	pid_t shm_lpid;
	unsigned long shm_nattch;
	unsigned long __pad1;
	unsigned long __pad2;
};

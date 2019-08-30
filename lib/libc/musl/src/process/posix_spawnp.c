#include <spawn.h>
#include <unistd.h>

int __execvpe(const char *, char *const *, char *const *);

int __posix_spawnx(pid_t *restrict, const char *restrict,
	int (*)(const char *, char *const *, char *const *),
	const posix_spawn_file_actions_t *,
	const posix_spawnattr_t *restrict, char *const *restrict, char *const *restrict);

int posix_spawnp(pid_t *restrict res, const char *restrict file,
	const posix_spawn_file_actions_t *fa,
	const posix_spawnattr_t *restrict attr,
	char *const argv[restrict], char *const envp[restrict])
{
	return __posix_spawnx(res, file, __execvpe, fa, attr, argv, envp);
}

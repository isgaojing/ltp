/*
 * Copyright (c) 2013 Oracle and/or its affiliates. All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it would be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write the Free Software Foundation,
 * Inc.,  51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
 *
 * Author: Alexey Kodanev <alexey.kodanev@oracle.com>
 *
 */

#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>
#include "test.h"

#define OPEN_MODE	(S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH)
#define OPEN_FLAGS	(O_WRONLY | O_APPEND | O_CREAT)

void tst_run_cmd_fds(void (cleanup_fn)(void),
		char *const argv[],
		int stdout_fd,
		int stderr_fd)
{
	if (argv == NULL || argv[0] == NULL) {
		tst_brkm(TBROK, cleanup_fn,
			"argument list is empty at %s:%d", __FILE__, __LINE__);
	}

	pid_t pid = vfork();
	if (pid == -1) {
		tst_brkm(TBROK | TERRNO, cleanup_fn, "vfork failed at %s:%d",
			__FILE__, __LINE__);
	}
	if (!pid) {
		/* redirecting stdout and stderr if needed */
		if (stdout_fd != -1) {
			close(STDOUT_FILENO);
			dup2(stdout_fd, STDOUT_FILENO);
		}

		if (stderr_fd != -1) {
			close(STDERR_FILENO);
			dup2(stderr_fd, STDERR_FILENO);
		}

		_exit(execvp(argv[0], argv));
	}

	int ret = -1;
	if (waitpid(pid, &ret, 0) != pid) {
		tst_brkm(TBROK, cleanup_fn, "waitpid failed at %s:%d",
			__FILE__, __LINE__);
	}

	if (!WIFEXITED(ret) || WEXITSTATUS(ret) != 0) {
		tst_brkm(TBROK, cleanup_fn, "failed to exec cmd '%s' at %s:%d",
			argv[0], __FILE__, __LINE__);
	}
}

void tst_run_cmd(void (cleanup_fn)(void),
		char *const argv[],
		const char *stdout_path,
		const char *stderr_path)
{
	int stdout_fd = -1;
	int stderr_fd = -1;

	if (stdout_path != NULL) {
		stdout_fd = open(stdout_path,
				OPEN_FLAGS, OPEN_MODE);

		if (stdout_fd == -1)
			tst_resm(TWARN | TERRNO,
				"open() on %s failed at %s:%d",
				stdout_path, __FILE__, __LINE__);
	}

	if (stderr_path != NULL) {
		stderr_fd = open(stderr_path,
				OPEN_FLAGS, OPEN_MODE);

		if (stderr_fd == -1)
			tst_resm(TWARN | TERRNO,
				"open() on %s failed at %s:%d",
				stderr_path, __FILE__, __LINE__);
	}

	tst_run_cmd_fds(cleanup_fn, argv, stdout_fd, stderr_fd);

	if ((stdout_fd != -1) && (close(stdout_fd) == -1))
		tst_resm(TWARN | TERRNO,
			"close() on %s failed at %s:%d",
			stdout_path, __FILE__, __LINE__);

	if ((stderr_fd != -1) && (close(stderr_fd) == -1))
		tst_resm(TWARN | TERRNO,
			"close() on %s failed at %s:%d",
			stderr_path, __FILE__, __LINE__);
}
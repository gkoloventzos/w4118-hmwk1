#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include "shell.h"

int main(int argc, char **argv)
{
	char *buf;
	char ***all = NULL;
	char *token;
	char **path = NULL;
	size_t line_length = 0;
	int llength = 0;
	int pipes;
	int i, dd, error;

	for (;;) {
		pipes = 0;
		error = 0;
		i = 0;
		all = calloc(1, sizeof(char ***));
		printf("$");
		if (getline(&buf, &line_length, stdin) == -1) {
			printf("\n");
			exit(0);
		}
		dd = strlen(buf);
		if (buf[dd - 2] == '|') {
			printf("error: dangling pipe\n");
			continue;
		}
		char *buf2 = strdup(buf);
		token = strtok(buf, "|");
		while (token != NULL) {
			all = realloc(all, (pipes + 1) * sizeof(char **));
			if (all == NULL) {
				printf("error: realloc failed\n");
				continue;
			}
			all[pipes] = calloc(128, sizeof(char *));
			if (all[pipes] == NULL) {
				printf("error: calloc failed\n");
				continue;
			}
			/*The next 6 lines are due to a bug in the
			   KR parser I am using for inputs like
			   ls -l| wc-l
			   when the pipe has no space with the previous
			   command */
			char *token2;
			int gg = strlen(token);

			i += gg;
			token2 = strdup(token);
			token2 = realloc(token2, gg + 2);
			token2[gg] = ' ';
			token2[gg + 1] = '\0';
			parse(token2, all[pipes]);
			pipes++;
			int z = dagling_pipe(buf2, i, dd);
			if (z) {	/*to catch the || */
				error = 1;	/*bash executes the part */
				break;	/*that works */
			}
			token = strtok(NULL, "|");
			i++;
		}
		if (error) {
			printf("error: error with pipes\n");
			continue;
		}
		execute(all, &path, &llength, pipes);
		if (llength == -1) {
			free(buf);
			break;
		}
		for (i = 0; i < pipes; i++)
			free(all[i]);
		free(all);
		free(buf2);
	}
	return 0;
}

int dagling_pipe(char *buf, int i, int ll)
{

	int z;
	for (z = i + 1; z < ll; z++) {
		if (buf[z] == '|') {
			return 1;
		} else if ((buf[z] != ' ') && (buf[z] != '\t')) {
			return 0;
		} else {
			continue;
		}
	}
	return 0;
}

void parse(char *buf, char **args)
{
	while (*buf != '\0') {
		while ((*buf == ' ') || (*buf == '\t') || (*buf == '\n'))
			*buf++ = '\0';
		*args++ = buf;
		while ((*buf != '\0') && (*buf != ' ') && (*buf != '\t') &&
		       (*buf != '\n'))
			buf++;
	}
	*--args = NULL;
}

void execute(char ***args, char ***path, int *ll, int pipes)
{
	int n = -1;
	int l = -1;
	int x = -1;
	int f, p, ret, external;
	int **pip;
	pid_t *kids;

	kids = calloc(pipes, sizeof(pid_t));
	if (kids == NULL) {
		printf("error: calloc failed\n");
		return;
	}
	pip = calloc(pipes, sizeof(int *));
	if (pip == NULL) {
		printf("error: calloc failed\n");
		return;
	}
	ret = 0;
	for (p = 0; p < pipes; p++) {
		pip[p] = malloc(2 * sizeof(int));
		if (pip[p] == NULL) {
			perror("error ");
			return;
		}
		pip[p][STDIN_FILENO] = -1;
		pip[p][STDOUT_FILENO] = -1;
	}
	for (p = 0; p < pipes - 1; p++) {
		int temp[2];

		ret = pipe(temp);
		if (ret == -1) {
			perror("error ");
			break;
		}
		pip[p][STDOUT_FILENO] = temp[STDOUT_FILENO];
		pip[p + 1][STDIN_FILENO] = temp[STDIN_FILENO];
	}
	if (ret == -1)
		return;
	for (p = 0; p < pipes; p++) {
		external = 0;
		if (args[p][0] == NULL)
			break;

		n = strcmp(args[p][0], "path");
		l = strcmp(args[p][0], "exit");
		x = strcmp(args[p][0], "cd");
		if (l == 0) {
			if (*ll > 0) {
				for (f = 0; f < (*ll); f++)
					free((*path)[f]);
				free(*path);
			}
			*ll = -1;
			continue;
		}
		if (n == 0) {
			mypath(args[p], path, ll);
			continue;
		}
		if (x == 0 && args[p][1] != NULL) {
			if (chdir(args[p][1]) == -1)
				perror("error");
			continue;
		}
		if (x == 0 && args[p][1] == NULL) {
			printf("No directory specified\n");
			continue;
		}
		int found = 0;
		char *exec_path;
		struct stat sb;

		if (args[p][0][0] != '/') {
			exec_path = create_path(args[p][0], path, ll);
		} else {
			exec_path = strdup(args[p][0]);
		}
		if (stat(exec_path, &sb) != -1) {
			found = 1;
			external = 1;
			kids[p] = my_fork(exec_path, args[p], pip, p, pipes);
			free(exec_path);
			continue;
		}

		if (!found) {
			printf("error: %s is not in the path\n", args[p][0]);
			continue;
		}

		free(exec_path);

	}

	for (p = 0; p < pipes; p++) {
		int stat, ret;

		if (pip[p][STDIN_FILENO] >= 0) {
			ret = close(pip[p][STDIN_FILENO]);
			if (ret == -1)
				perror("error ");
		}
		if (pip[p][STDOUT_FILENO] >= 0) {
			close(pip[p][STDOUT_FILENO]);
			if (ret == -1)
				perror("error ");
		}
		if (external) {
			ret = waitpid(kids[p], &stat, 0);
			if (ret == -1)
				perror("error ");
		}
	}
	free(kids);
}

void mypath(char **args, char ***path, int *leng)
{

	int i;

	if (args[1] == NULL) {
		if ((*leng) == 0)
			return;
		path_print(*path, *leng, 1);
		return;
	}
	if (args[1] == NULL || args[2] == NULL) {
		printf("error: Use of path: path [+|- absolute path]\n");
		return;
	}
	if (strncmp(args[1], "-", 1) == 0) {
		int find = -1;
		int f;
		char *temp;

		for (f = 0; f < (*leng); f++) {
			if (find < 0 && strcmp(args[2], (*path)[f]) == 0)
				find = f;
			if (find >= 0 && f < ((*leng) - 1)) {
				temp = (*path)[f];
				(*path)[f] = strdup((*path)[f + 1]);
				free(temp);
			}
		}
		if (find == -1) {
			printf("Path: %s has not been added.\n", args[2]);
			return;
		}
		free((*path)[((*leng) - 1)]);
		(*leng)--;
		if (*leng == 0)
			free(*path);
		return;
	}
	if (strncmp(args[1], "+", 1) == 0) {
		for (i = 0; i < *leng; i++) {
			if (strcmp((*path)[i], args[2]) == 0)
				return;
		}
		(*path) =
		    (char **)realloc((*path), ((*leng) + 1) * sizeof(char *));
		if (*path == NULL) {
			printf("error: realloc failed");
			return;
		}
		(*path)[(*leng)] = strdup(args[2]);
		(*leng)++;
		return;
	}
	/*Any other symbol */
	printf("error: Use of path: path [+|- absolute path]\n");
}

void path_print(char **path, int length, int in)
{

	int i;

	if (in) {
		for (i = 0; i < length; i++) {
			printf("%s", path[i]);
			if (i < length - 1)
				printf(":");
		}
		printf("\n");
		return;
	}
	for (i = 0; i < length; i++)
		printf("i:%d string:%s\n", i, path[i]);

}

pid_t my_fork(char *cmd, char **args, int **pipes, int pipe_num, int all_pipes)
{

	pid_t pid;
	int m, ret;

	pid = fork();
	if (pid != 0)
		return pid;

	if (pipes[pipe_num][STDIN_FILENO] >= 0) {
		ret = dup2(pipes[pipe_num][STDIN_FILENO], STDIN_FILENO);
		if (ret == -1)
			perror("error ");
	}
	if (pipes[pipe_num][STDOUT_FILENO] >= 0) {
		ret = dup2(pipes[pipe_num][STDOUT_FILENO], STDOUT_FILENO);
		if (ret == -1)
			perror("error ");
	}

	for (m = 0; m < all_pipes; m++) {
		if (pipes[m][STDIN_FILENO] >= 0) {
			ret = close(pipes[m][STDIN_FILENO]);
			if (ret == -1)
				perror("error ");
		}
		if (pipes[m][STDOUT_FILENO] >= 0) {
			ret = close(pipes[m][STDOUT_FILENO]);
			if (ret == -1)
				perror("error ");
		}
	}

	execv(cmd, args);
	perror("error");
	return -1;

}

char *create_path(char *args, char ***path, int *ll)
{
	int f;
	char *exec_path;

	for (f = 0; f < (*ll); f++) {
		if ((*path) == NULL) {
			printf
			    ("error: path is null\nPlease add some path first.\n");
			break;
		}
		size_t s_l = strlen((*path)[f]);

		if ((*path)[f][s_l] != '/') {
			size_t ii = strlen((*path)[f]) + strlen(args) + 2;

			exec_path = calloc(ii, sizeof(char));
			strcat(exec_path, (*path)[f]);
			strcat(exec_path, "/");
			strcat(exec_path, args);
		} else {
			size_t ii = strlen((*path)[f]) + strlen(args) + 1;

			exec_path = calloc(ii, sizeof(char));
			strcat(exec_path, (*path)[f]);
			strcat(exec_path, args);
		}
	}
	return exec_path;
}

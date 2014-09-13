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
	int i, error;

	for (;;) {
		pipes = 0;
		error = 0;
		all = calloc(1, sizeof(char ***));
		printf("$ ");
		if (getline(&buf, &line_length, stdin) == -1) {
			printf("\n");
			exit(0);
		}
		token = strtok(buf, "|");
		while (token != NULL) {
			all = realloc(all, (pipes + 1) * sizeof(char **));
			all[pipes] = calloc(128, sizeof(char *));
			/*The next 6 lines are due to a bug in the
			   KR parser I am using for inputs like
			   ls -l| wc-l
			   when the pipe has no space with the previous
			   command */
			char *token2;

			token2 = strdup(token);
			int gg = strlen(token2);

			token2 = realloc(token2, gg + 2);
			token2[gg] = ' ';
			token2[gg + 1] = '\0';
			parse(token2, all[pipes]);
			pipes++;
			if (token[gg + 1] == '|') {	/*to catch the || */
				error = 1;	/*bash executes tha part that */
				break;      /*works*/
			}
			token = strtok(NULL, "|");
		}
		if (error)
			continue;
		execute(all, &path, &llength, pipes);
		if (llength == -1) {
			free(buf);
			break;
		}
		for (i = 0; i < pipes; i++)
			free(all[i]);
		free(all);
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
	pip = calloc(pipes, sizeof(int *));
	ret = 0;
	for (p = 0; p < pipes; p++) {
		pip[p] = malloc(2 * sizeof(int));
		pip[p][STDIN_FILENO] = -1;
		pip[p][STDOUT_FILENO] = -1;
	}
	for (p = 0; p < pipes - 1; p++) {
		int temp[2];

		ret = pipe(temp);
		if (ret == -1) {
			perror("error: ");
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
				perror("Error");
			continue;
		}
		if (x == 0 && args[p][1] == NULL) {
			printf("No directory specified\n");
			continue;
		}
		int found = 0;
		char *exec_path;
		struct stat sb;

		printf("%s\n", args[p][0]);
		if (args[p][0][0] != '/') {
			for (f = 0; f < (*ll); f++) {
				if ((*path) == NULL) {
					printf
					    ("Error: path is null\nPlease add some path first.\n");
					break;
				}
				size_t s_l = strlen((*path)[f]);

				if ((*path)[f][s_l] != '/') {
					size_t ii =
					    strlen((*path)[f]) +
					    strlen(args[p][0]) + 2;

					exec_path = calloc(ii, sizeof(char));
					strcat(exec_path, (*path)[f]);
					strcat(exec_path, "/");
					strcat(exec_path, args[p][0]);
				} else {
					size_t ii =
					    strlen((*path)[f]) +
					    strlen(args[p][0]) + 1;

					exec_path = calloc(ii, sizeof(char));
					strcat(exec_path, (*path)[f]);
					strcat(exec_path, args[p][0]);
				}
			}
		} else {
			exec_path = strdup(args[p][0]);
		}
		if (stat(exec_path, &sb) != -1) {
			found = 1;
			external = 1;
			kids[p] = my_fork(exec_path, *args, pip, p, pipes);
			free(exec_path);
			continue;
		}
		free(exec_path);

		if (!found)
			printf("error: %s is not in the path\n", args[p][0]);

		continue;
	}

	if (external) {
		for (p = 0; p < pipes; p++) {
			int stat, ret;

			if (pip[p][STDIN_FILENO] >= 0)
				close(pip[p][STDIN_FILENO]);
			if (pip[p][STDOUT_FILENO] >= 0)
				close(pip[p][STDOUT_FILENO]);
			ret = waitpid(kids[p], &stat, 0);
			/*fprintf(stderr, "Child %d returned %d\n",
			   m, WEXITSTATUS(stat)); */
			if (ret == -1)
				perror("error: ");
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
			if (find < 0 && strcmp(args[2], (*path)[f]) == 0) {
				find = f;
				if (f == (*leng) - 1) {
					(*leng)--;
					free((*path)[f]);
					return;
				}
			}
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
		(*path)[(*leng)] = strdup(args[2]);
		(*leng)++;
		return;
	}

}

void path_print(char **path, int length, int in)
{

	int i;

	if (in) {
		printf("PATH=");
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
	int m;

	pid = fork();
	if (pid == 0) {
		if (pipes[pipe_num][STDIN_FILENO] >= 0)
			dup2(pipes[pipe_num][STDIN_FILENO], STDIN_FILENO);
		if (pipes[pipe_num][STDOUT_FILENO] >= 0)
			dup2(pipes[pipe_num][STDOUT_FILENO], STDOUT_FILENO);

		for (m = 0; m < all_pipes; m++) {
			if (pipes[m][STDIN_FILENO] >= 0)
				close(pipes[m][STDIN_FILENO]);
			if (pipes[m][STDOUT_FILENO] >= 0)
				close(pipes[m][STDOUT_FILENO]);
		}

		execv(cmd, args);
		perror("error:");
		return -1;
	}

	return pid;
}

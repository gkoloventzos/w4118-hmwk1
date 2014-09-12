#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>

void parse(char *buf, char **args);
void execute(char ***args, char ***path, int *len, int pipes);
void mypath(char **args, char ***path, int *len);
void path_print(char **path, int len, int in_line);

int main(int argc, char **argv)
{
	char *buf;
    char ***all = NULL;
    char *token;
	char **path = NULL;
    size_t line_length = 0;
    int llength = 0;
    int pipes;
    int i;

	for (;;) {
        pipes = 0;
        all = calloc(1,sizeof(char ***));
		printf("$ ");
		if (getline(&buf, &line_length, stdin) == -1) {
			printf("\n");
            exit(0);
        }
        token = strtok(buf, "|");
	    while (token != NULL) {
                all = realloc(all, (pipes+1)*sizeof(char**));
		        all[pipes] = calloc(128,sizeof(char*));
                /*The next 6 lines are due to a bug in the KR parser I am using for inputs like
                ls -l| wc-l
                when the pipe has no space with the command*/
                char *token2;
                token2 = strdup(token);
                int gg = strlen(token2);
                token2 = realloc(token2,gg+2);
                token2[gg] = ' ';
                token2[gg+1] = '\0';
                parse(token2, all[pipes]);
		        pipes++;
                token = strtok(NULL, "|");
	    }
		execute(all, &path, &llength, pipes);
	    if (llength == -1) {
		    free(buf);
		    break;
	    }
        for (i=0;i<pipes;i++) {
            free(all[i]);
        }
/*        if (pipes == 0)
            free(all[0]);*/
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
		while ((*buf != '\0') && (*buf != ' ') && (*buf != '\t') && \
			(*buf != '\n'))
			buf++;
    }
		*--args = NULL;
}

void execute(char ***args, char ***path, int *ll, int pipes)
{
	int pid, status;
	int n = -1;
	int l = -1;
    int x = -1;
    int f,p;

    for (p=0;p<pipes;p++) {
    if (args[0][0] == NULL) {
	continue;
    }
	n = strcmp(args[p][0], "path");
	l = strcmp(args[p][0], "exit");
    x = strcmp(args[p][0], "cd");
	if ((*path) == NULL) {
		if (n != 0 && l != 0 && x != 0) {
			printf("Error: path is null\nPlease add some path fir\
st.\n");
			continue;
		}
	}
	if (l == 0) {
	if (*ll > 0) {
            for (f = 0; f < (*ll); f++) {
		free((*path)[f]);
	    }
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
        if (chdir(args[p][1]) == -1) {
            perror("Error");
        }
        continue;
    }
    for (f = 0; f < (*ll); f++) {
	    char *exec_path;
        size_t ii = strlen((*path)[f]) + strlen(args[p][0]) + 1;
	    struct stat sb;

	    exec_path = calloc(ii,sizeof(char));
        strcat(exec_path, (*path)[f]);
        strcat(exec_path, args[p][0]);
        if (stat(exec_path, &sb) != -1) {
            if ((pid = fork()) < 0) {
                perror("Error");
            }
            if (pid == 0) {
                execv(exec_path, *args);
                perror(exec_path);
            }
            if (wait(&status) != pid) {
                perror("Error");
            }
            free(exec_path);
            continue;
	    }
	    free(exec_path);
    }
    continue;
    }
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
		printf("Use of path: path [+|- absolute path]\n");
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
	    if (find >= 0 && f < ((*leng)-1)) {
		temp = (*path)[f];
                (*path)[f] = strdup((*path)[f+1]);
		free(temp);
	    }
	}
	if (find == -1) {
            printf("Path: %s has not been added.\n", args[2]);
	    return;
	}
	free((*path)[((*leng)-1)]);
	(*leng)--;
	if (*leng == 0)
	    free(*path);
		return;
	}
	if (strncmp(args[1], "+", 1) == 0) {
        for (i = 0; i <  *leng; i++) {
            if (strcmp((*path)[i], args[2]) == 0)
		return;
	}
        (*path) = (char **)realloc((*path), ((*leng)+1)*sizeof(char*));
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
    for (i = 0; i < length; i++) {
        printf("i:%d string:%s\n", i, path[i]); }

}

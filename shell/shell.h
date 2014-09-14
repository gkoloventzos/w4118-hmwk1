#include <stdio.h>

void parse(char *buf, char **args);
void execute(char ***args, char ***path, int *len, int pipes);
void mypath(char **args, char ***path, int *len);
char * create_path(char *args, char ***path, int *len);
void path_print(char **path, int len, int in_line);
pid_t my_fork(char *cmd, char **args, int **pipes, int pipe_num, int allpipes);

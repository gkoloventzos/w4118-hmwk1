#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
void parse(char *buf, char **args);
void execute(char **args, char ** path);
void mypath(char **args, char ** path);

int main()
{ 
	char buf[2048]; char *args[128];
	char **path = NULL;
	for (;;) {
		printf("$");
		if (fgets(buf, sizeof(buf), stdin) == NULL) {
			printf("\n"); exit(0); }
			parse(buf, args);
			execute(args, path); 
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
			buf++; }
		*--args = NULL; 
}

void execute(char **args, char **path)
{ 
	int pid, status;
	int n = -1;
	if (path == NULL){
		n = strcmp(args[0],"path");
		n = strcmp(args[0],"exit");
		if ( n != 0){
			printf("Error: path is null\nPlease add some path fir\
st.\n");
			return;
		}
	}
	if ( n == 0)
		exit(0);
	if ((pid = fork()) < 0) {
		perror("fork");
	}
	if (pid == 0) {
		execvp(*args, args);
		perror(*args);
	}
	if (wait(&status) != pid) {
		perror("wait");
	} 
}

void mypath(char **args, char **path){

	char *path_new;
	char *ppath = NULL;
	int i;
	size_t len;
	if (args[1] == NULL){
		if (path == NULL)
			return;
		path_new=path[0];
		for(i=0; path_new != NULL; i++){
		printf("here\n");
			path_new = path[i];
			ppath = realloc(ppath,strlen(path_new)+1);
			len = strlen(ppath);
			ppath[len]=":";
		}
		if (ppath != NULL){
			len = strlen(ppath);
                	ppath[len]="\0";
		}
		printf("before print\n");
		printf("PATH=%s",ppath);
		return;
	}
	
}

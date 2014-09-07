#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

void parse(char *buf, char **args);
void execute(char **args, char *** path);
void mypath(char **args, char *** path);

int main()
{
	char buf[2048]; char *args[128];
	char **path = NULL;
	for (;;) {
		printf("$ ");
		if (fgets(buf, sizeof(buf), stdin) == NULL) {
			printf("\n"); exit(0); }
			parse(buf, args);
			execute(args, &path);
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

void execute(char **args, char ***path)
{ 
	int pid, status;
	int n = -1;
	int l = -1;
	n = strcmp(args[0],"path");
	l = strcmp(args[0],"exit");
	if ((*path) == NULL){
		if (n != 0 && l != 0){
			printf("Error: path is null\nPlease add some path fir\
st.\n");
			return;
		}
	}
	if (l == 0){
		//free path
		exit(0);
	}
	if (n == 0){
		mypath(args, path);
		return;
	}
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

void mypath(char **args, char ***path){

	char *path_new;
	char *ppath = NULL;
	int i,z;
	size_t len;
	if (args[1] == NULL){
		if ((*path) == NULL){
			return;}
		path_new=(*path)[0];
			printf("here\n");
		while (path_new != NULL){
			if (*ppath == NULL){
				size_t d = strlen(path_new);
				ppath = malloc(d+2);
				strcpy(ppath,path_new);
				ppath[d+1] = ":";
				ppath[d+2] = "\0";
			}
			else{
				z = strlen(ppath);
				ppath = realloc(ppath,z+strlen(path_new)+2);
			}
			if (ppath == NULL){
				printf("Error in locating space for print path.\n");
				return;
			}
			
		}
		for(i=0; path_new != NULL; i++){
			printf("here\n");
			path_new = (*path)[i];
			ppath = realloc(ppath,strlen(path_new)+1);
			len = strlen(ppath);
			ppath[len]=":";
		}
		if (ppath != NULL){
			printf("here\n");
			len = strlen(ppath);
                	ppath[len]="\0";
		}
			printf("there\n");
		printf("PATH=%s",ppath);
		return;
	}
	if (args[1] == NULL || args[2] == NULL){
		printf("Use of path: path [+|- absolute path]\n");
		return;
	}	
	if (strncmp(args[1],"-",1) == 0){
		return;
	}
	if (strncmp(args[1],"+",1) == 0){
		if ((*path) == NULL){
			(* path) = (char **)calloc(1,sizeof(char*));
			if ((*path) == NULL){
				printf("Error in locating space for new path!\n");
			}
			(*path)[0] = strdup(args[2]);
			if ((*path)[0] == NULL){
				printf("Error in locating space for new path!\n");
			}
		}
		return;
	}
	
}

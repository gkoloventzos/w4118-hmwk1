#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

void parse(char *buf, char **args);
void execute(char **args, char *** path, int *len);
void mypath(char **args, char *** path, int *len);
void path_print(char **path, int len, int in_line);

int main()
{
	char *buf; char *args[128];
	char **path = NULL;
    size_t line_length = 0;
    int llength = 0;
	for (;;) {
		printf("$ ");
		if (getline(&buf, &line_length, stdin) == -1) {
			printf("\n"); exit(0); }
			parse(buf, args);
			execute(args, &path,&llength);
            if (llength == -1){
                free(buf);
                //free path
                //free args
                break;
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
		while ((*buf != '\0') && (*buf != ' ') && (*buf != '\t') && \
			(*buf != '\n'))
			buf++; }
		*--args = NULL; 
}

void execute(char **args, char ***path, int *ll)
{ 
	int pid, status;
	int n = -1;
	int l = -1;
    if (args[0] == NULL){
        return;
    }
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
        *ll = -1;
		//free path
		return;
	}
	if (n == 0){
		mypath(args, path, ll);
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

void mypath(char **args, char ***path, int *leng){

	if (args[1] == NULL){
		if ((*leng) == 0)
			return;
		path_print(*path,*leng,1);
		return;
	}
	if (args[1] == NULL || args[2] == NULL){
		printf("Use of path: path [+|- absolute path]\n");
		return;
	}	
	if (strncmp(args[1],"-",1) == 0){
        int find = -1;
        int f;
        for (f = 0; f <(*leng); f++){
            if (strcmp(args[2],(*path)[f]) == 0){
                find =f;
                if (f == (*leng) -1){
                    (*leng)--;
                    free((*path)[f]);
                    return ;
                }
            }
            if (find > 0 && f < ((*leng)-1)){
                printf("f:%d\n",f);
                free((*path)[f]);
                (*path)[f] = strdup((*path[f+1]));
            }
        }
        if (find == -1){
            printf("Path: %s is not added.\n",args[2]);
            return;
        }
        free((*path)[((*leng)--)]);
        (*leng)--;
		return;
	}
	if (strncmp(args[1],"+",1) == 0){
        (* path) = (char **)realloc((* path),((*leng)+1)*sizeof(char*));
        (* path)[(*leng)] = strdup(args[2]);
        (*leng)++;
        path_print(*path,*leng,0);
		return;
	}
	
}

void path_print(char **path, int length, int in){

    int i;
    if (in){
        printf("PATH=");
        for (i=0; i<length;i++){
            printf("%u",path[i]);
            if (i < length -1)
                printf(":");
        }
        printf("\n");
        return;
    }
        printf("path:%u\n",path);
    for (i=0; i<length;i++){
        printf("i:%d string:%u\n",i,path[i]);}

}

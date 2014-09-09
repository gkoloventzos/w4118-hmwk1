#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

void parse(char *buf, char **args);
void execute(char **args, char *** path, int *len);
void mypath(char **args, char *** path, int *len);

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

	char *path_new;
	char *ppath = NULL;
	int i,z,path_length;
	size_t len;
	if (args[1] == NULL){
		if ((*leng) == 0){
			return;}
        path_length = *leng;
        path_length--;
        i = 0;
		do{
            path_new = (*path)[i];
			if (ppath == NULL){
				size_t d = strlen(path_new);
				ppath = (char*) malloc((d+2)*sizeof(char));
			    if (ppath == NULL){
				    printf("Error in locating space for print path.\n");
				    return;
			    }
				strcpy(ppath,path_new);
				ppath[d+1] = ':';
				ppath[d+2] = '\0';
    
			}
			else{
				z = strlen(ppath);
				ppath = realloc(ppath,(z+strlen(path_new)+2)*sizeof(char));
			    if (ppath == NULL){
				    printf("Error in locating space for print path.\n");
				    return;
			    }
                strcat(ppath,":");
                strcat(ppath,path_new);
			}
			i++;
		}while (i <= path_length);
		/*for(i=0; path_new != NULL; i++){
			printf("here\n");
			path_new = (*path)[i];
			ppath = realloc(ppath,strlen(path_new)+1);
			len = strlen(ppath);
			ppath[len] = ':';
		}*/
		if (ppath != NULL){
			len = strlen(ppath);
                	ppath[len] = '\0';
		}
		printf("PATH=%s\n",ppath);
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
                    free((*path)[f]);
                    (*leng)--;
                }
            }
        }
        if (find == -1){
            printf("Path: %s is not added.\n",args[2]);
            return;
        }
		return;
	}
	if (strncmp(args[1],"+",1) == 0){
		if ((*leng) == 0){
			(* path) = (char **)calloc(1,sizeof(char*));
			if ((*path) == NULL){
				printf("Error in locating space for new path!\n");
			}
			(*path)[0] = strdup(args[2]);
			if ((*path)[0] == NULL){
				printf("Error in locating space for new path!\n");
			}
            (*leng)++;
            return;
		}
        (* path) = (char **)realloc((* path),((*leng)+1)*sizeof(char*));
        (* path)[(*leng)] = strdup(args[2]);
        (*leng)++;
		return;
	}
	
}

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
void parse(char *buf, char **args);
void execute(char **args);
main()
{ 
	char buf[1024]; char *args[64];
	for (;;) {
		printf("Command: ");
		if (fgets(buf, sizeof(buf), stdin) == NULL) {
			printf("\n"); exit(0); }
			parse(buf, args);
			execute(args); 
	}
}
void parse(char *buf, char **args)
{ 
	while (*buf != ’\0’) {
		while ((*buf == ’ ’) || (*buf == ’\t’) || (*buf == ’\n’))
			*buf++ = ’\0’;
		*args++ = buf;
		while ((*buf != ’\0’) && (*buf != ’ ’) &&
		(*buf != ’\t’) && (*buf != ’\n’))
			buf++; }
		*--args = NULL; 
}
void execute(char **args)
{ 
	int pid, status;
	if ((pid = fork()) < 0) {
		perror("fork"); exit(1); 
	}
	if (pid == 0) {
		execvp(*args, args); perror(*args); exit(1); 
	}
	if (wait(&status) != pid) {
		perror("wait"); exit(1); 
	} 
}

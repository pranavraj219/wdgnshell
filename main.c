#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define LINE_BUFF_SZ 64
#define NARGS_BUFF_SZ 8
#define ARGS_TOK_DELM " "

int wdgn_cd(char **args);
int wdgn_help(char **args);
int wdgn_exit(char **args);

int (*wdgn_builtin_func[]) (char**) = {&wdgn_cd, &wdgn_help, &wdgn_exit};

char *wdgn_builtin_funcnames[] = {"cd", "help", "exit"};

int wdgn_sz_builtins(){
	return sizeof(wdgn_builtin_funcnames)/sizeof(char*);
}

int wdgn_cd(char **args){
	if(!args[1]){
		fprintf(stderr, "wdgnsh: Expected argument to command 'cd'\n");
	}
	else{
		if(chdir(args[1]) != 0){
			perror("wdgnsh");
		}
	}
	return 1;
}

int wdgn_exit(char **args){
	return 0;
}

int wdgn_help(char **args){
	printf("\n\tWDgn Shell -- created by Pranav Raj (WDragon)\n");
	printf("Builtin Functions -\n");
	int sz = wdgn_sz_builtins();
	for(int i = 0; i < sz; ++i){
		printf(" %s\n", wdgn_builtin_funcnames[i]);
	}
	printf("Other Commands same as the system functions\n\n");
	return 1;
}


int execExternal(char **args){
	pid_t pid, wpid;
	int status;
	pid = fork();
	if(pid == 0){
		if(execvp(args[0], args) == -1){
			perror("wdgnsh");				// Prints this before any error;
		}
		exit(EXIT_FAILURE);					// While we can exit with a non-zero code... but this looks better
	}
	else if(pid < 0){
		perror("wdgnsh");
	}
	else{
		do{
			wpid = waitpid(pid, &status, WUNTRACED);
		}while(!WIFEXITED(status) && !WIFSIGNALED(status));
	}
	return 1;
}

int execCommand(char **args){
	if(!args[0]){
		return 1;
	}
	int sz = wdgn_sz_builtins();
	for(int i = 0; i < sz; ++i){
		if(strcmp(args[0], wdgn_builtin_funcnames[i]) == 0){
			return (*wdgn_builtin_func[i])(args);
		}
	}
	return execExternal(args);
}


char **splitLine(char *line){
	char **args = (char**)malloc(NARGS_BUFF_SZ * sizeof(char*));
	if(!args){
		fprintf(stderr, "ARGUMENT POINTER MEMORY CANNOT BE ALLOCATED\n");
	}
	int idx = 0;
	char *arg = strtok(line, ARGS_TOK_DELM);
	int currArgSize = NARGS_BUFF_SZ;
	args[0] = arg;
	arg = strtok(NULL, ARGS_TOK_DELM);
	while(arg){
		++idx;
		if(idx >= currArgSize){
			currArgSize += NARGS_BUFF_SZ;
			args = realloc(args, currArgSize * sizeof(char*));
			fprintf(stderr, "ARGUMENT POINTER MEMORY CANNOT BE REALLOCATED\n");

		}
		args[idx] = arg;
		arg = strtok(NULL, ARGS_TOK_DELM);
	}
	++idx;
	args[idx] = NULL;
	return args;
}


char *readLine(){
	char *line = (char*)malloc(LINE_BUFF_SZ * sizeof(char));
	if(!line){
		fprintf(stderr, "CMD MEMORY CANNOT BE ALLOCATED\n");
	}
	int currBuffSize = LINE_BUFF_SZ, idx = 0;
	char ch;
	while((ch = getchar()) && ch != EOF && ch != '\n'){
		if(idx >= currBuffSize){
			currBuffSize += LINE_BUFF_SZ;
			line = realloc(line, currBuffSize * sizeof(char));
			if(!line){
				fprintf(stderr, "CMD MEMORY CANNOT BE REALLOCATED\n");
			}
		}
		line[idx] = ch;
		++idx;
	}
	line[idx] = '\0';
	return line;
}

void shell_Repeat(){
	char *cmd;
	char **args;
	int retStatus;
	do{
		printf("wdgnsh> ");
		cmd = readLine();
		args = splitLine(cmd);
		retStatus = execCommand(args);
		free(cmd);
		free(args);
	}while(retStatus);
}


int main(int argc, char **argv){
	printf("\n\n\t**** Welcome to WDgn Shell (wdgnsh) ****\n\n");
	shell_Repeat();
	return 0;
}
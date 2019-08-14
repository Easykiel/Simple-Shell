#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>

#include "sshell.h"

/*checks if input is valid*/
int checkInput(char *input, struct command *commands, int numCMD){
	int i, j;

	if(strlen(input) == 2 && !isalpha(input[0]))
		return 1;

	for(i = 0; i < numCMD + 1; i++){
		for(j = 0; j < ALL_CMDS; j++){
			if(strcmp(commands[i].args[0], commandList[j]) == 0)
				return 0;
		}
		return -1;
	}

	return 0;
}

/*Handles all errors*/
void handleErrors(int status){
	fprintf(stderr, "Error: ");

	switch (status){
	case 1:         //Invalid Command Line
		fprintf(stderr, "invalid command line\n");
		break;

	case -1:         //Command not found
		fprintf(stderr, "command not found\n");
		break;

	case 2:		//No input File
		fprintf(stderr, "no input file\n");
		break;

	case 3:		//No output File
		fprintf(stderr, "no output file\n");
		break;
	}
}

/*Checks for input and output Redirection*/
int checkRedirection(char input, struct command *cmd){
	if(input == '<'){
		cmd->command_flags = INPUT_REDIRECTION;
		return 1;
	}

	if(input == '>'){
		cmd->command_flags = OUTPUT_REDIRECTION;
		return 1;
	}

	return 0;
}

/*Gets input from the user*/
void getInput(char* input, struct command *commands, int * numCMD){
	char temp[MAX_BUFF_SIZE];
	int i = 0, k = 0, l = 0, numArgs = 0;

	fgets(input, MAX_BUFF_SIZE, stdin);
	/*Splits inputs into the command.cmd buffer*/
	while(input[i] != '\0')
	{
		temp[k] = input[i++];
		if(temp[k] == '|'){
			commands[(*numCMD)].args[numArgs] = NULL;
			(*numCMD)++;
			numArgs = 0;
		}
		if(temp[k] == '&')
			commands[(*numCMD)].command_flags = WAIT;	
		if(temp[k] == '<')
			commands[(*numCMD)].command_flags = INPUT_REDIRECTION;	
		if(temp[k] == '>')
			commands[(*numCMD)].command_flags = OUTPUT_REDIRECTION;	

		if(temp[k] != ' ' && temp[k] != '<' && temp[k] != '>' && temp[k] != '\n' && temp[k] != '|' && temp[k] != '&')
			k++;

		else if(k>0){
			temp[k] = '\0';
			commands[(*numCMD)].args[numArgs] = (char *)malloc(strlen(temp) * sizeof(char));
			strcpy(commands[(*numCMD)].args[numArgs++], temp);
			memset(temp, 0, MAX_BUFF_SIZE);
			k = 0; 

			if(checkRedirection(input[i], &commands[(*numCMD)]) == 1){
				i++;

				while(isspace(input[i]))
					i++;

				while(!isspace(input[i])){
					commands[(*numCMD)].File[l++] = input[i++];
				}
				l = 0;
			}
		}
	}
	commands[(*numCMD)].args[numArgs] = NULL;
}

/*For Input Redirection*/
void inputRedirect(char* file){
	int fd;

	fd = open(file,O_RDONLY);
	dup2(fd, STDIN_FILENO);
	close(fd);
	memset(file, 0, MAX_FILE_SIZE);
}

/*For Output Redirection*/
void outputRedirect(char* file){
	int fd;

	fd = open(file, O_RDWR | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	dup2(fd, STDOUT_FILENO);
	close(fd);
	memset(file, 0, MAX_FILE_SIZE);
}

/*Handles Built in Commands*/
int builtinCommand(char** command){
	char cwd[256];

	if(strcmp(command[0], "exit") == 0){
		fprintf(stderr, "Bye...\n");
		exit(1);
	}

	else if (strcmp(command[0], "cd") == 0){
		if(chdir(command[1]) == -1)
			fprintf(stderr, "Error: no such directory\n");
		return 1;
	}

	else if(strcmp(command[0], "pwd") == 0){
		getcwd(cwd, sizeof(cwd));
		fprintf(stdout, "%s\n", cwd);
		return 1;
	}

	return 0;
}


/*For piping multiple processes*/
void pipeline(struct command *cmd1){
	int fd[2];

	pipe(fd);
	if(fork() == 0){
		//Parent
		close(fd[0]);
		dup2(fd[1], STDOUT_FILENO);
		close(fd[1]);
		execvp(cmd1->args[0], cmd1->args);
	} else{
		close(fd[1]);
		dup2(fd[0], STDIN_FILENO);
		close(fd[0]);
	}
}

/*Initializes the command structs*/
void initializeCMD(struct command *cmd, int * numCMD, int * status){
	int numCMDS;

	(*numCMD) = 0;
	for(numCMDS = 0; numCMDS < MAX_ARG_SIZE; numCMDS++){
		cmd[numCMDS].command_flags = NOTHING;
		cmd[numCMDS].File[0] = '\0';
	}
	memset(cmd, 0, MAX_ARG_SIZE * sizeof(struct command));
	memset(status, 0, MAX_ARG_SIZE * sizeof(int));
}

/*Prints the completion message*/
void printCompletion(char * input, int * status, int numCMD){
	int i;

	strtok(input, "\n");
	fprintf(stderr, "+ completed '%s' ", input);
	for(i = 0; i < numCMD + 1; i++)
		fprintf(stderr, "[%d]", WEXITSTATUS(status[i]));
	fprintf(stderr, "\n");
}

int main(int argc, char *argv[])
{
	pid_t pid;

	struct command commands[MAX_ARG_SIZE];
	int isError, numCMD, i;
	int status[MAX_ARG_SIZE];
	char input[MAX_BUFF_SIZE];


	while(1){
		/*Initialize CMD flags*/
		initializeCMD(commands, &numCMD, status);
		waitpid(-1, NULL, WNOHANG);
		printf("sshell$ ");
		fflush(stdout);
		
		/* Gets Command Line then Prints to STDOUT*/
		getInput(input, commands, &numCMD);
		if(!isatty(STDIN_FILENO)){
			printf("%s", input);
			fflush(stdout);
		}

		// Checks If there are Error in input
		isError = checkInput(input, commands, numCMD);

		if(isError){ //If there are Errors in Input 
			handleErrors(isError);
			
			if(isError == 1)
				continue;
		}

		if(builtinCommand(commands[0].args)){
			status[0] = 0;
			printCompletion(input, status, numCMD);
			continue;
		}
		if(strcmp(commands[0].File, "") == 0 &&
commands[0].command_flags == INPUT_REDIRECTION){
			handleErrors(2);
			continue;
		}

		if(strcmp(commands[0].File, "") == 0 && commands[0].command_flags ==
OUTPUT_REDIRECTION){
			handleErrors(3);
			continue;
		}

		//Splits then forks into
			pid = fork();
			if (pid == 0){ //Child

				if(commands[0].command_flags == INPUT_REDIRECTION)
					inputRedirect(commands[0].File);

				if(commands[numCMD].command_flags == OUTPUT_REDIRECTION)
					outputRedirect(commands[numCMD].File);

				if(commands[0].command_flags == WAIT)
					setpgid(0, 0);

				if(numCMD > 0){
					for(i = 0; i < numCMD; i++)
						pipeline(&commands[i]);

					for(i = 0; i < numCMD; i++)
						wait(&status[i]);
				}
				execvp(commands[numCMD].args[0], commands[numCMD].args);
				exit(2);
			} else if (pid > 0){ //Parent
				if(commands[0].command_flags != WAIT)
					wait(&status[0]);
			} else { //Failed
				fprintf(stderr, "fork failed\n");
				exit(1);
			}
		printCompletion(input, status, numCMD);
	}
	return EXIT_SUCCESS;
}

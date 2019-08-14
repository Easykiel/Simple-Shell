#ifndef _SSHELL_H_
#define _SSHELL_H_

#define MAX_BUFF_SIZE 512
#define MAX_ARG_SIZE 16
#define MAX_FILE_SIZE 30
#define MAX_STRING_SIZE 100
#define ALL_CMDS 12

enum {NOTHING, INPUT_REDIRECTION, OUTPUT_REDIRECTION, WAIT};

struct command{
	char* args[MAX_ARG_SIZE];
	char File[MAX_FILE_SIZE];
	int command_flags;
};

char *commandList[] = {"wc","sleep", "mkdir", "grep", "ls", "pwd", "cd", "date", "exit", "cat", "echo", "touch"};

int checkInput(char* input, struct command *commands, int numCMD);

int checkRedirection(char input, struct command* cmd);

void handleErrors(int status);

void getCMD(char* input, char* cmd[]);

void getInput(char* input, struct command* command, int * numCMD);

void inputRedirect(char* file);

void outputRedirect(char* file);

int builtinCommand(char** command);

void pipeline(struct command *cmd1);

void initializeCMD(struct command *cmd, int *numCMD, int *status);

void printCompletion(char *input, int *status, int numCMD);
#endif

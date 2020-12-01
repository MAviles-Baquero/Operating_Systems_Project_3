/*
 Maria Elena Aviles-Baquero
 CPSC 346: Project 3
 UNIX Shells and History Feature
 
 To Compile: cc -o exec_simple simple_shell.c
 */

#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>


#define MAX_LINE 80 /* 80 chars per line, per command, should be enough. */
#define MAX_COMMANDS 9 /* size of history */

char history[MAX_COMMANDS][MAX_LINE]; //the array used to store history commands.
/*the array used for "printf" to display history nicely. Remove special characters like "\n" or "\0"*/

int command_count = 0;

/**
 * Add the most recent command to the history.
 */

void addtohistory(char inputBuffer[], char *args[]) {
	int hist_count = 0;
	int i, j, k;
	
	if(strcmp(args[0], "history") == 0) {
		if(command_count > 0) {
			hist_count = command_count;
			
			for(j = 0; j < MAX_COMMANDS; j++) {
				k = 0;
				printf("%d\t", hist_count);
				while(history[j][k]!='\n' && history[j][k]!='\0') {
					
					printf("%c", history[j][k]);
					k++;
				}
				printf("\n");
				hist_count--;
				if(hist_count == 0)
					break;
			} /*end of for loop*/
			printf("\n");
		} /*end of inner if statement*/
		else {
			printf("\nNo Commands in History\n");
		}
		exit(-1);
	} /*end of outer if statement*/
	
	else if(args[0][0] - '!' == 0) {
		int a = args[0][1] - '0';
		
		if(a > command_count) {
			printf("\nNo Specified Command Found. Enter a number smaller than MAX_COMMANDS (9)\n");
			strcpy(inputBuffer, "Wrong command");
		}
		else {
			if(a == -15) 
				strcpy(inputBuffer, history[0]);	// last command typed
			else if (a == 0) {
				printf("Enter a number in range.\n");
				strcpy(inputBuffer, "Wrong command");
			}
			else if(a >= 1) 
				strcpy(inputBuffer, history[command_count-a]);
		}
	}
	else if(strcmp(args[0], "exit") == 0){
		exit(0);
	}
	for(i = MAX_COMMANDS; i > 0; i--) {
		strcpy(history[i], history[i-1]);
	}
	strcpy(history[0], inputBuffer);
	command_count++;
	if(command_count > MAX_COMMANDS) 
		command_count = MAX_COMMANDS;
	
	return;
}


/** 
 * The setup function below will not return any value, but it will just: read
 * in the next command line; separate it into distinct arguments (using blanks as
 * delimiters), and set the args array entries to point to the beginning of what
 * will become null-terminated, C-style strings. 
 */

int setup(char inputBuffer[], char *args[], int *background) {
	int length;		/* # of characters in the command line */
	int i, j, k;				/* loop index for accessing inputBuffer array */
	int command_number;	/* index of requested command number */

	//define your local variables here;
	int start;
	int hist_count = 0;
	
    /* read what the user enters on the command line */
	do {
		printf("osh>");
		fflush(stdout);
		length = read(STDIN_FILENO,inputBuffer,MAX_LINE); 
	}
	while (inputBuffer[0] == '\n'); /* swallow newline characters */

	start = -1;
	
	if (length == 0)
		exit(0);            /* ^d was entered, end of user command stream */
		
	if (length < 0) {
		perror("ERROR reading the command");
		exit(-1);           /* terminate with error code of -1 */
	}
	
	/**
	 * Parse the contents of inputBuffer
	 */
	for(i = 0; i < length; i++) {
		/* examine every character in the inputBuffer */
		switch(inputBuffer[i]) {
			case ' ':
			case '\t':	/* argument separators */
				if(start != -1) {
					args[command_count] = &inputBuffer[start];
					command_count++;
				}
				inputBuffer[i] = '\0'; // null character
				start = -1;
				break;
				
			case '\n':	/* should be the final char examined */
				if(start != -1) {
					args[command_count] = &inputBuffer[start];
					command_count++;
				}
				inputBuffer[i] = '\0';
				args[command_count] = NULL;
				break;
			
			default:	/* some other character */
				if(start == -1)
					start = i;
				if(inputBuffer[i] == '&') {
					*background = 1;
					inputBuffer[i] = '\0';
				}
		} /* end of switch */
	} /* end of for loop */
	
	args[command_count] = NULL;	// if the line was greater than MAX_LINE
	
	/**
	 * Add the command to the history
	*/
	addtohistory(inputBuffer, args);
	
	
	return 1;	
} /* end of setup routine */



int main(void)
{
	char inputBuffer[MAX_LINE]; 	/* buffer to hold the command entered */
	int background;             	/* equals 1 if a command is followed by '&' */
	char *args[MAX_LINE/2 + 1];	/* command line (of 80) has max of 40 arguments */
	pid_t child;            		/* process id of the child process */
	
	//define your local variables here, at the beginning of your program. 
	int i;
	int shouldrun = 1;
	int status;
		
    while (shouldrun){	/* Program terminates normally inside setup */
		background = 0;
		
		shouldrun = setup(inputBuffer,args,&background);	/* get next command */
		
		if (shouldrun) {
			/* creates a duplicate process! */
			child = fork();
			
			if(child < 0) {	/*pid < 0, error*/
				printf("Fork failed.\n");
				exit(1);
			}
			
			else if(child == 0) {	/*pid == 0, it is the child process*/
				if(execvp(args[0], args) == -1) 
					printf("Error in execution.\n");
			}
			
			else {	/*pid > 0, it is the parent*/
				i++;
				if(background == 0) {
					i++;
					waitpid(child, &status, 0);
				}
			}
		}	/*end of if statement*/
	}
	return 0;
} /*end of main function*/

/*END OF CODE*/


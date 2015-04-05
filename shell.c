/*
 * Code for execution of commands
 */
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>

#include "parser.h"
#include "shell.h"

/**
 * Program that simulates a simple shell.
 * The shell covers basic commands, including builtin commands 
 * (cd and exit only), standard I/O redirection and piping (|). 
 
 */

#define MAX_DIRNAME 100
#define MAX_COMMAND 1024
#define MAX_TOKEN 128

/* Functions to implement, see below after main */
int execute_cd(char** words);
int execute_nonbuiltin(simple_command *s);
int execute_simple_command(simple_command *cmd);
int execute_complex_command(command *cmd);


int main(int argc, char** argv) {
	
	char cwd[MAX_DIRNAME];           /* Current working directory */
	char command_line[MAX_COMMAND];  /* The command */
	char *tokens[MAX_TOKEN];         /* Command tokens (program name, 
					  * parameters, pipe, etc.) */

	while (1) {

		/* Display prompt */		
		getcwd(cwd, MAX_DIRNAME-1);
		printf("%s> ", cwd);
		
		/* Read the command line */
		fgets(command_line, MAX_COMMAND, stdin);
		/* Strip the new line character */
		if (command_line[strlen(command_line) - 1] == '\n') {
			command_line[strlen(command_line) - 1] = '\0';
		}
		
		/* Parse the command into tokens */
		parse_line(command_line, tokens);

		/* Check for empty command */
		if (!(*tokens)) {
			continue;
		}
		
		/* Construct chain of commands, if multiple commands */
		command *cmd = construct_command(tokens);
		//print_command(cmd, 0);
    
		int exitcode = 0;
		if (cmd->scmd) {
			exitcode = execute_simple_command(cmd->scmd);
			if (exitcode == -1) {
				break;
			}
		}
		else {
			exitcode = execute_complex_command(cmd);
			if (exitcode == -1) {
				break;
			}
		}
		release_command(cmd);
	}
    
	return 0;
}


/**
 * Changes directory to a path specified in the words argument;
 * For example: words[0] = "cd"
 *              words[1] = "csc209/assignment3/"
 * Your command should handle both relative paths to the current 
 * working directory, and absolute paths relative to root,
 * e.g., relative path:  cd csc209/assignment3/
 *       absolute path:  cd /u/bogdan/csc209/assignment3/
 */
int execute_cd(char** words) {
	
	/** 
	 * TODO: 
	 * The first word contains the "cd" string, the second one contains 
	 * the path.
	 * Check possible errors:
	 * - The words pointer could be NULL, the first string or the second 
	 *   string could be NULL, or the first string is not a cd command
	 * - If so, return an EXIT_FAILURE status to indicate something is 
	 *   wrong.
	 */
    
    //Check to see if arguments are correct.
    //char * command="cd";
    if(words[0]==NULL || strcmp("cd", words[0])!=0){
        exit(EXIT_FAILURE);
    }
    if(words[1]==NULL){
        exit(EXIT_FAILURE);
    }
    //Checks if path is relative
    char currdir[MAX_DIRNAME];
    int relative = is_relative(words[1]);
    int result = relative;
    
    //Check to see if words[1] is too long
    //If words[1] is too long exits(fail).
    if (strlen(words[1]) > MAX_DIRNAME) {
        exit(EXIT_FAILURE);
    }

    //If path is relative, concatenate it current path and change dir.
    if(is_relative(words[1]) == 1)
    {
        getcwd(currdir, MAX_DIRNAME);
        strncat(strncat(currdir, "/", MAX_DIRNAME), words[1], MAX_DIRNAME);
        chdir(currdir);
        result = 0;
    }
    //If Path is absolute
    else
    {
        // simply executes chdir and changes directory to absolute path.
        chdir(words[1]);
        result = 0;
    }
    
    return(result);
    
    
    
    
    


	/**
	 * TODO: 
	 * The safest way would be to first determine if the path is relative 
	 * or absolute (see is_relative function provided).
	 * - If it's not relative, then simply change the directory to the path 
	 * specified in the second word in the array.
	 * - If it's relative, then make sure to get the current working 
	 * directory, append the path in the second word to the current working
	 * directory and change the directory to this path.
	 * Hints: see chdir and getcwd man pages.
	 * Return the success/error code obtained when changing the directory.
	 */
	 
}


/**
 * Executes a program, based on the tokens provided as 
 * an argument.
 * For example, "ls -l" is represented in the tokens array by 
 * 2 strings "ls" and "-l", followed by a NULL token.
 * The command "ls -l | wc -l" will contain 5 tokens, 
 * followed by a NULL token. 
 */
int execute_command(char **tokens) {
	
	/**
	 * TODO: execute a program, based on the tokens provided.
	 * The first token is the command name, the rest are the arguments 
	 * for the command. 
	 * Hint: see execlp/execvp man pages.
	 * 
	 * - In case of error, make sure to use "perror" to indicate the name
	 *   of the command that failed.
	 *   You do NOT have to print an identical error message to what would 
	 *   happen in bash.
	 *   If you use perror, an output like: 
	 *      my_silly_command: No such file of directory 
	 *   would suffice.
	 * Function returns only in case of a failure (EXIT_FAILURE).
	 */
    int result;
    result=0;
    result=execvp(tokens[0], tokens);
    if(result==-1){
        perror(tokens[0]);
    }
    
    return result;

}


/**
 * Executes a non-builtin command.
 */
int execute_nonbuiltin(simple_command *s){
	/**
	 * TODO: Check if the in, out, and err fields are set (not NULL),
	 * and, IN EACH CASE:
	 * - Open a new file descriptor (make sure you have the correct flags,
	 *   and permissions);
	 * - redirect stdin/stdout/stderr to the corresponding file.
	 *   (hint: see dup2 man pages).
	 * - close the newly opened file descriptor in the parent as well. 
	 *   (Avoid leaving the file descriptor open across an exec!) 
	 * - finally, execute the command using the tokens (see execute_command
	 *   function above).
	 * This function returns only if the execution of the program fails.
	 */
    
    // New file descriptors with default values in case they are NULL
    int in_des=0;
    int out_des=1;
    int err_des=2;
    //Checking if they are NULL and initialising the descriptors
    // and runnign dup2
    if (s->in != NULL) {
        in_des = open(s->in, O_TRUNC);
        dup2(in_des, 0);
        close(in_des);
    }
    
    if (s->out != NULL) {
        out_des = open(s->out, O_RDWR|O_CREAT, 0777);
        dup2(out_des, 1);
        close(out_des);
    }
    
    if (s->err != NULL) {
        err_des = open(s->err, O_TRUNC);
        dup2(err_des, 2);
        close(err_des);
    }
    
    
    // Executes command
    return(execute_command(s->tokens));
    
    
    }


/**
 * Executes a simple command (no pipes).
 */
int execute_simple_command(simple_command *cmd) {
    
    // CD
    if(cmd->builtin==1){
        execute_cd(cmd->tokens);
        return 0;
    }
    //Exit
    if(cmd->builtin==2){
        exit(EXIT_SUCCESS);
        return 0;
    }
    // Not CD or Exit
    else{
        pid_t pid;
        pid = fork();
        int status;
        if (pid > 0){ 			/* only parent gets here */
            if((wait(&status)) == -1) {
                perror("wait");
                exit(1);
            }
        
        } else if (pid == 0){		/* only child gets here */
            execute_nonbuiltin(cmd);
        } else{			/* error */
            perror("fork()");
            
        }
        
    }
    
    
    return 0;
	/**
	 * TODO: 
	 * Check if the command is builtin.
	 * 1. If it is, then handle BUILTIN_CD (see execute_cd function provided) 
	 *    and BUILTIN_EXIT (simply exit with an appropriate exit status).
	 * 2. If it isn't, then you must execute the non-builtin command. 
	 * - Fork a process to execute the nonbuiltin command 
	 *   (see execute_nonbuiltin function above).
	 * - The parent should wait for the child.
	 *   (see wait man pages).
	 */
	
}


/**
 * Executes a complex command.  A complex command is two commands chained 
 * together with a pipe operator.
 */
int execute_complex_command(command *c) {
	
	/**
	 * TODO:
	 * Check if this is a simple command, using the scmd field.
	 * Remember that this will be called recursively, so when you encounter
	 * a simple command you should act accordingly.
	 * Execute nonbuiltin commands only. If it's exit or cd, you should not 
	 * execute these in a piped context, so simply ignore builtin commands. 
	 */
    if(c->scmd!=NULL){
        execute_nonbuiltin(c->scmd);
    }
    

	/** 
	 * Optional: if you wish to handle more than just the 
	 * pipe operator '|' (the '&&', ';' etc. operators), then 
	 * you can add more options here. 
	 */

	if (!strcmp(c->oper, "|")) {
		
		/**
		 * TODO: Create a pipe "pfd" that generates a pair of file 
		 * descriptors, to be used for communication between the 
		 * parent and the child. Make sure to check any errors in 
		 * creating the pipe.
		 */
        int pfd[2], pid, pid2;
        int status1;
        int status2;
        if((pipe(pfd)) == -1) {
            perror("pipe error");
            exit(EXIT_FAILURE);
        }
			
		/**
		 * TODO: Fork a new process.
		 * In the child:
		 *  - close one end of the pipe pfd and close the stdout 
		 * file descriptor.
		 *  - connect the stdout to the other end of the pipe (the 
		 * one you didn't close).
		 *  - execute complex command cmd1 recursively. 
		 * In the parent: 
		 *  - fork a new process to execute cmd2 recursively.
		 *  - In child 2:
		 *     - close one end of the pipe pfd (the other one than 
		 *       the first child), and close the standard input file 
		 *       descriptor.
		 *     - connect the stdin to the other end of the pipe (the 
		 *       one you didn't close).
		 *     - execute complex command cmd2 recursively. 
		 *  - In the parent:
		 *     - close both ends of the pipe. 
		 *     - wait for both children to finish.
		 */
		
        if((pid = fork()) > 0){
            //Parent of 1st fork process
            
            if((pid2 = fork()) > 0){
                //Parent of fork 2
                //Close both ends of pipe pfd
                close(pfd[0]);
                close(pfd[1]);
                //Waits for both children to finish
                wait(&status2);
                wait(&status1);
                return EXIT_SUCCESS;
            }
            
            else if(pid2 == 0){
                //2nd Child
                //Close other end of pipe
                close(pfd[1]);
                //redirect stdin
                dup2(pfd[0], fileno(stdin));
                //close file descriptor
                close(pfd[0]);
                // execute command
                execute_complex_command(c->cmd2);
                //exit
                exit(0);
            }
            else if(pid2 == -1){
                perror("Fork");
                exit(EXIT_FAILURE);
            }
            
        }
        else if(pid == 0){
            //Child No. 1
            //Close one end of pipe
            close(pfd[0]);
            // redirect
            dup2(pfd[1], fileno(stdout));
            //close pipe file dexriptor 1.
            close(pfd[1]);
            //Executes command.
            execute_complex_command(c->cmd1);
            //Exits
            exit(0);
            
            }
        else{
            perror("fork error");
            exit(EXIT_FAILURE);
        }
        
        
	}
	return 0;
}






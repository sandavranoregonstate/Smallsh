#include <sys/types.h> 
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>

#define TOKEN_BUFFER_SIZE 512
#define TOKEN_DELIM " \t\r\n\a"

struct sigaction SIGINT_action = {0};
struct sigaction SIGTSTP_action = {0};

void toggle_foreground(int signo) ; 
void to_the_background_add(int id_no_process_the ) ; 
void to_the_background_remove(int id_no_process_the ) ; 
void pid_replace(char *source, const char *searchValue, const char *replaceValue) ; 
char* string_input() ; 
char** make_tokenize(char *line) ; 
int the_fork_and_exec(char **args) ; 
void execute_command(char **args) ; 
void do_the_background_processes() ; 
int main (int argc, char* argv[]) ; 

char *DIR;			
int in; 			
int out; 			
char *value_input;		
char *output;		
int status;			
int the_program_is_running = 1; 	
int fg_mode = 0;	
int is_the_process_at_background = 0;	
int background_process_id [64];		
int count_background = 0; 	

int the_fork_and_exec(char **args) {
	
	pid_t spawnPid;
	pid_t waitPid;
	
	spawnPid = fork(); 

	// some things has made the error in  
	
	if(spawnPid < 0) {
		
		perror("fork");
		exit(1);
	
	} 
    
	// if the value returned by the fork is this value then it is the child process 
    else if (spawnPid == 0) {

		fflush(stdout); 

		// if the flag has been set for the new input file descriptor 
		if (in) {  

			// open the input file descriptor 
			int the_first_file_descriptor = open(value_input, O_RDONLY); 

			// it is not possible to open this file descriptor 
			if (the_first_file_descriptor == -1) { 
				printf("cannot open %s for value_input\n", value_input);
				fflush(stdout);
				exit(1);
			} 
            
			// it is not possible to do this expected 
            else if (dup2(the_first_file_descriptor, STDIN_FILENO) == -1) { 
				perror("dup2");
				close(the_first_file_descriptor);	
			}  

			// successful expected 
			else { 
				close(the_first_file_descriptor); 
			} 

		} 

		// if the flag has been set for the new not input file descriptor 
		if (out) {  

			// open the input file descriptor 
			int the_not_first_descriptor  = creat(output, 0644); 

			// it is not possible to open this file descriptor 
			if (the_not_first_descriptor  == -1) {
				printf("cannot create %s for output\n", output);
				fflush(stdout);
				exit(1); 
			} 
			
			// it is not possible to do this expected 
			else if (dup2(the_not_first_descriptor , STDOUT_FILENO) == -1) {
				perror("dup2");
				close(the_not_first_descriptor );	
			} 

			// successful expected 
			else { 
				close(the_not_first_descriptor );
			} 

		} 

		// if the process is to be run in the background 
		if ( is_the_process_at_background ) { 

			// if the input has not yet been given do set the input to the /dev/null 
			
			if (!in) { 

				// open the input file descriptor 
				int the_first_file_descriptor = open("/dev/null", O_RDONLY); 


				// it is not possible to open this file descriptor 
				if (the_first_file_descriptor == -1) {
					printf("cannot set /dev/null to value_input\n");
					fflush(stdout);
					exit(1);
				} 
				
				// it is not possible to do this expected 
				else if (dup2(the_first_file_descriptor, STDIN_FILENO) == -1)  {
					perror("dup2");
					close(the_first_file_descriptor);	
				} 

				// successful expected 
				else { 
					close(the_first_file_descriptor);	
				} 

			} 
			
			// if the not input has not yet been given do set the input to the /dev/null 
			if (!out) { 

				// open the input file descriptor 
				int the_not_first_descriptor  = creat("/dev/null", 0644);

				// it is not possible to open this file descriptor 
				if (the_not_first_descriptor  == -1) {
					printf("cannot set /dev/null to output\n");
					fflush(stdout);
					exit(1);			
				} 
				
				// it is not possible to do this expected 
				else if (dup2(the_not_first_descriptor , STDOUT_FILENO) == -1) {
					perror("dup2");
					close(the_not_first_descriptor );	
				} 

				// successful expected 
				else { 
					close(the_not_first_descriptor ); 
				} 

			} 

		} 

		if (!is_the_process_at_background) {
			
			SIGINT_action.sa_handler = SIG_DFL;
			SIGINT_action.sa_flags = 0; 

			sigaction(SIGINT, &SIGINT_action, NULL); 

		} 

		execvp(args[0], args) ; 
		perror(args[0]); 

		exit(1); 

	} 

	// if the value returned by the fork is not this value then it is the parent process 
	else { 
		
		// if the process is not to be run in the background do wait for them to be done 
		if (!is_the_process_at_background) { 

			waitPid = waitpid(spawnPid, &status, WUNTRACED);

			if (waitPid == -1) { 
				perror("waitpid"); 
				exit(1); 
			}
				
			if (WIFSIGNALED(status)) {
				printf("terminated by signal %d\n", WTERMSIG(status));
				fflush(stdout);
			}
				
			if (WIFSTOPPED(status)) {
				printf("stopped by signal %d\n", WSTOPSIG(status));
				fflush(stdout);
			} 

			while (!WIFEXITED(status) && !WIFSIGNALED(status)) { 

				waitPid = waitpid(spawnPid, &status, WUNTRACED);

				if (waitPid == -1) { 
					perror("waitpid"); 
					exit(1); 
				}
					
				if (WIFSIGNALED(status)) {
					printf("terminated by signal %d\n", WTERMSIG(status));
					fflush(stdout);
				}
					
				if (WIFSTOPPED(status)) {
					printf("stopped by signal %d\n", WSTOPSIG(status));
					fflush(stdout);
				} 
			}
	
		} 
		
		// if the process is to be run in the background do not wait for them to be done  
		else {
			
			printf("background pid is %d\n", spawnPid); 
			fflush(stdout); 

			to_the_background_add(spawnPid);
			is_the_process_at_background = 0; // reset bg bool in parent 

		}
	} 

	return 0; 

}

void execute_command(char **args) { 

	if ((args[0] != NULL) && (strchr(args[0],'#') == NULL ) ) { 

		if (strcmp(args[0], "exit") == 0) {
			
			// send the required signal 
			while (count_background > 0) { 
				kill(background_process_id [0],SIGTERM); 
				to_the_background_remove(background_process_id [0]); 
			}

			the_program_is_running = 0;
		
		} 

		else if (strcmp(args[0], "cd") == 0) { 

			// if there is no second argument go to the home directory 
			if (args[1] == NULL) { 

				if( chdir(getenv("HOME")) != 0 ) {
					perror("chdir");
				} 

			} 

			// if there is the directory in second argument go to the directory provided   
			else { 
				if(chdir(args[1]) != 0) {
					perror("chdir");
				} 
			}	 

		} 

		else if (strcmp(args[0], "status") == 0) {	
		
			// the process has been exited naturally 
			if (WIFEXITED(status)) {
				printf("exit value %d\n", WEXITSTATUS(status));
				fflush(stdout);
			} 
			
			// the process has did exit not naturally via some signal 
			else if (WIFSIGNALED(status)){
				printf("terminating signal %d\n", WTERMSIG(status));
				fflush(stdout);	
			}
		} 

		else {

			// execute the remaining commands via the fork and exec programming method 
			the_fork_and_exec(args); 

		} 

	} 

} 

void do_the_background_processes() {
	
	// check for the background processes 
	pid_t background_process_id ;
	background_process_id  = waitpid( - 1 , &status, WNOHANG ) ;
	
	while ( background_process_id  > 0 ) {
		
		// do remove the background process id from the array of background processes 
		to_the_background_remove(background_process_id );
		
		// the program has exited normally 
		if ( 0 != WIFEXITED(status) ) { 
			printf("background pid %d is done. exit value %d\n", background_process_id , WEXITSTATUS(status)); 
			fflush(stdout);
		} 
		
		// the program has been exited with a signal 
		else if ( 0 !=WIFSIGNALED(status) ) {
			printf("background pid %d is done. terminated by signal %d\n", background_process_id , WTERMSIG(status) ) ;  
			fflush(stdout);	
		}
		
		// check for the background processes 
		background_process_id  = waitpid(-1, &status, WNOHANG); 

	}	
} 

int main (int argc, char* argv[]) {
	
	DIR = getenv("PWD"); 

	SIGINT_action.sa_handler = SIG_IGN;
	sigaction(SIGINT, &SIGINT_action, NULL);

	SIGTSTP_action.sa_handler = toggle_foreground;
	sigfillset(&SIGTSTP_action.sa_mask); 

	SIGTSTP_action.sa_flags = SA_RESTART; 
	
	sigaction(SIGTSTP, &SIGTSTP_action, NULL); 
	
	char *line;
	char **args;
	
	while( the_program_is_running ) { 

        // find out if there are processes in the background 
		if ( count_background > 0) {	
			do_the_background_processes();
		}
		
        // use the read line method to read the value_input line 
		line = string_input(); 

        // use the parse line method to parse the read line 
		args = make_tokenize(line); 

		execute_command(args);
        // make sure there is no memory leaks that do happen 

		if ( NULL == line ) { 
			// 
		} 
		else {
			free(line); 
		} 

		if ( NULL == line ) { 
			// 
		} 
		else {
			free(args); 
		} 

		value_input = NULL; 
		output = NULL; 

	} 

	chdir(DIR); 

	return 0; 

} 

void to_the_background_add(int id_no_process_the ) {
	
    background_process_id [count_background] = id_no_process_the ;
	count_background++;	 

} 

void to_the_background_remove(int id_no_process_the ) {
	
	int i; 
    for (i = 0; i < count_background; i++) {
		if (background_process_id [i] == id_no_process_the ) {
			while (i < count_background - 1) {
				background_process_id [i] = background_process_id [i + 1];
				i++;
			}
			count_background--;
			break;
		}
    } 
} 

char* string_input() { 

	// input value  
	
	printf(": "); 
	fflush(stdout); 
	
	char *line = NULL;
	ssize_t value_buffer = 0;  

	getline(&line, &value_buffer, stdin); 
	char *p = strstr(line, "$$"); 

	// if the $$ is found do extend it with given process id the 
	if (p) {
		char pidStr[6];
		sprintf(pidStr, "%d", getpid());
		pid_replace(line, "$$", pidStr); 
	} 

	return line; 

} 

void pid_replace(char *source, const char *searchValue, const char *replaceValue) {

    size_t searchValue_len = strlen(searchValue); 		
    size_t replaceValue_len = strlen(replaceValue); 	
	char buffer[2048] = { 0 };					

    char *insert_point = &buffer[0];					
    const char *tmp = source;							

	int the_counter = 1 ; 
	
    while ( the_counter ) { 

		// if there is the $$ in the string 
        const char *p = strstr(tmp, searchValue);

        if (p == NULL) {
            strcpy(insert_point, tmp);
            break;
        }

        memcpy(insert_point, tmp, p - tmp);
        insert_point += p - tmp;

        memcpy(insert_point, replaceValue, replaceValue_len);
        insert_point += replaceValue_len;

        tmp = p + searchValue_len; 

    } 

	// send the buffer to the value source 
    strcpy(source, buffer); 

} 

char** make_tokenize(char *line) {
	
	char **tokens = malloc(TOKEN_BUFFER_SIZE * sizeof(char*)); 
	char *token; 
	int position = 0;
	in = 0;
	out = 0;
	
	token = strtok(line, TOKEN_DELIM); 
		
	while (token != NULL) { 

		// this is the not input given possible redirection 		
		if (strcmp(token, ">") == 0) {
			out = 1;
			output = strtok(NULL, TOKEN_DELIM);
			token = strtok(NULL, TOKEN_DELIM);
			tokens[position] = NULL;
			position++;
			continue;
		}
		
		// this is the input given possible redirection 		
		if (strcmp(token, "<") == 0) {
			in = 1;
			value_input = strtok(NULL, TOKEN_DELIM);
			token = strtok(NULL, TOKEN_DELIM);
			tokens[position] = NULL;
			position++;
			continue;
		}
		
		if (strcmp(token, "&") == 0) { 

			tokens[position] = NULL; 

			// if we are at foreground only mode 
			if (fg_mode) {
				is_the_process_at_background = 0;
			} 

			// if we are not at the foreground only mode 
			else {
				is_the_process_at_background = 1;
			} 

			break; 

		}			
		
		tokens[position] = token; 
		position++;
		token = strtok(NULL, TOKEN_DELIM); 

	}
	
	tokens[position] = NULL;
	return tokens; 

} 

void toggle_foreground(int signo) { 

	if (fg_mode == 0) {	
		char *message = "\nEntering foreground-only mode (& is now ignored)\n";
		write(STDOUT_FILENO, message, 50);
		fg_mode = 1; 
	} 
	
	else {
		char *message = "\nExiting foreground-only mode\n";
		write(STDOUT_FILENO, message, 30);
		fg_mode = 0; 
	} 

	char *prompt = ": "; 
	write(STDOUT_FILENO, prompt, 2); 

} 


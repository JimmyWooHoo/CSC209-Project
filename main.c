#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node.h"

#define LINE_LENGTH 256

int main(int argc, char **argv) {
	// Declare any new variables you need
	if (argc < 2) {
		fprintf(stderr, "Not enough arguments");
		exit(1);
	}

	FILE* files = fopen(argv[1], "r");
	// open file and error check
	if (files == NULL) {
		fprintf(stderr, "Error opening file\n");
		exit(1);
	}
	
	// find the number of filenames are given
	char line[LINE_LENGTH + 1];
	int len = 0;
	while(fgets(line, LINE_LENGTH + 1, files) != NULL) {
		len++;		
	}
	
	char **filenames = malloc(sizeof(char*) * len);
	rewind(files);
	
	int i = 0;
	while(fgets(line, LINE_LENGTH + 1, files) != NULL) {
		line[strcspn(line, "\n")] = '\0';
		filenames[i] = malloc(strlen(line) + 1);
		strncpy(filenames[i], line, strlen(line) + 1);
		i++;		
	}
	
	int fd[len][2];

	// then call pipe, and then fork so all children have this array as well
	// child writes to the pipe, parent reads from the pipe
	for (int j = 0; j < len; j++) {
 		if (pipe(fd[j]) == -1) {
			perror("pipe");
			exit(1);
		}
	
		int result = fork();
		if (result < 0) {
			perror("fork");
			exit(1);	
		} else if (result == 0) {
			// Child process only writes to the process, close reading end
			close (fd[j][0]);
			for (int k = 0; k < j; k++) {
				close (fd[k][0]);
			}
			
			// Now we can start making the word index for the child process
			// parse it into an array of words?
			char **word_list = read_words(filenames[j]);
			Node *node_list = generate_node_family(word_list);
			
			// Now we can send this word frequency to the parent process
			
			char *output = convert_node_family(node_list);
			write(fd[j][1], output, strlen(output));
			free(output);
			close(fd[j][1]);
			exit(0);
		} else {
			// close the end of the pipe in the parent process 
			// we don't want open
			close(fd[j][1]);
		}	
	}	
	 // only the parent gets here
	return 0;
}



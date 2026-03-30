#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <unistd.h>
#include "node.h"

#define LINE_LENGTH 256
#define READ_CHUNK 256

// function for alphabetical sort
int compare_alphabetical(const void *a, const void *b) {
	Node *nodeA = *(Node**)a;
	Node *nodeB = *(Node**)b;

	return strcmp(nodeA->word, nodeB->word);
}

// function for reversed alphabetical sort
int compare_alphabetical_reversed(const void *a, const void *b) {
	Node *nodeA = *(Node**)a;
	Node *nodeB = *(Node**)b;

	return strcmp(nodeB->word, nodeA->word);
}

// function for high to low frequency sort
int compare_frequency(const void *a, const void *b) {
	Node *nodeA = *(Node**)a;
	Node *nodeB = *(Node**)b;

	if (nodeA->count != nodeB->count) {
		return nodeB->count - nodeA->count;
	}
	return strcmp(nodeA->word, nodeB->word);
}

// function for low to high frequency sort
int compare_frequency_reversed(const void *a, const void *b) {
	Node *nodeA = *(Node**)a;
	Node *nodeB = *(Node**)b;

	if (nodeA->count != nodeB->count) {
		return nodeA->count - nodeB->count;
	}
	return strcmp(nodeA->word, nodeB->word);
}
static void free_filenames(char **filenames, int count) {
	for (int i = 0; i < count; i++) {
		free(filenames[i]);
	}
	free(filenames);
}

static void merge_child_results(Node **global_list, const char *buffer) {
	// Parse one child's "word count" lines and fold them into the global list.
	char *copy = malloc(strlen(buffer) + 1);
	if (copy == NULL) {
		perror("malloc");
		exit(1);
	}
	strcpy(copy, buffer);

	char *line = strtok(copy, "\n");
	while (line != NULL) {
		char word[80];
		int count;

		if (sscanf(line, "%79s %d", word, &count) == 2) {
			Node *found = find_node(*global_list, word);
			if (found == NULL) {
				Node *new = new_node(word);
				if (new == NULL) {
					perror("malloc");
					free(copy);
					exit(1);
				}
				new->count = count;
				new->next = *global_list;
				*global_list = new;
			} else {
				found->count += count;
			}
		}

		line = strtok(NULL, "\n");
	}

	free(copy);
}

static char *read_from_pipe(int fd) {
	// Grow the buffer as needed so one child can send back any amount of text.
	size_t capacity = READ_CHUNK + 1;
	size_t used = 0;
	char *buffer = malloc(capacity);
	if (buffer == NULL) {
		perror("malloc");
		exit(1);
	}

	while (1) {
		ssize_t bytes_read = read(fd, buffer + used, capacity - used - 1);
		if (bytes_read < 0) {
			perror("read");
			free(buffer);
			exit(1);
		}
		if (bytes_read == 0) {
			break;
		}

		used += (size_t) bytes_read;
		// Double the buffer once it fills up to keep reads simple.
		if (used == capacity - 1) {
			capacity *= 2;
			char *grown = realloc(buffer, capacity);
			if (grown == NULL) {
				perror("realloc");
				free(buffer);
				exit(1);
			}
			buffer = grown;
		}
	}

	buffer[used] = '\0';
	return buffer;
}

int main(int argc, char **argv) {
	// Declare any new variables you need
	if (argc < 2) {
		fprintf(stderr, "Usage: %s [-f | -a | -rf | -ra] <file list>\n", argv[0]);
		exit(1);
	}
	int (*sort_func)(const void *, const void *) = compare_frequency;
	char *filename = NULL;

	for (int i = 1; i < argc; i++) {
		if (strcmp(argv[i], "-f") == 0) {
			sort_func = compare_frequency;
		} else if (strcmp(argv[i], "-rf") == 0) {
			sort_func = compare_frequency_reversed;
		} else if (strcmp(argv[i], "-a") == 0) {
			sort_func = compare_alphabetical;
		}else if (strcmp(argv[i], "-ra") == 0) {
			sort_func = compare_alphabetical_reversed;
		}else {
			filename = argv[i];
		}
	}

	if (filename == NULL) {
		fprintf(stderr, "Error: no file list provided.\n");
		exit(1);
	}

	FILE *files = fopen(filename, "r");
	// open file and error check
	if (files == NULL) {
		perror("fopen");
		exit(1);
	}

	char line[LINE_LENGTH + 1];
	int len = 0;
	// find the number of filenames are given
	while (fgets(line, LINE_LENGTH + 1, files) != NULL) {
		len++;
	}

	if (len == 0) {
		fclose(files);
		return 0;
	}

	char **filenames = malloc(sizeof(char *) * len);
	if (filenames == NULL) {
		perror("malloc");
		fclose(files);
		exit(1);
	}

	rewind(files);
	for (int i = 0; i < len; i++) {
		if (fgets(line, LINE_LENGTH + 1, files) == NULL) {
			fprintf(stderr, "Error reading filename list\n");
			free_filenames(filenames, i);
			fclose(files);
			exit(1);
		}

		line[strcspn(line, "\n")] = '\0';
		filenames[i] = malloc(strlen(line) + 1);
		if (filenames[i] == NULL) {
			perror("malloc");
			free_filenames(filenames, i);
			fclose(files);
			exit(1);
		}
		strcpy(filenames[i], line);
	}
	fclose(files);

	int fd[len][2];
	pid_t child_pids[len];

	// then call pipe, and then fork so all children have this array as well
	// child writes to the pipe, parent reads from the pipe
	for (int j = 0; j < len; j++) {
		if (pipe(fd[j]) == -1) {
			perror("pipe");
			free_filenames(filenames, len);
			exit(1);
		}

		pid_t result = fork();
		if (result < 0) {
			perror("fork");
			free_filenames(filenames, len);
			exit(1);
		} else if (result == 0) {
			// Child process only writes to the process, close reading end
			close(fd[j][0]);
			for (int k = 0; k < j; k++) {
				close(fd[k][0]);
			//	close(fd[k][1]);
			}

			// Now we can start making the word index for the child process
			char **word_list = read_words(filenames[j]);
			Node *node_list = generate_node_family(word_list);
			if (node_list == NULL && word_list[0] != NULL) {
				deallocate_words(word_list);
				close(fd[j][1]);
				exit(1);
			}

			char *output = convert_node_family(node_list);
			if (output == NULL) {
				perror("malloc");
				deallocate_nodes(node_list);
				deallocate_words(word_list);
				close(fd[j][1]);
				exit(1);
			}

			// Now we can send this word frequency to the parent process
			if (write(fd[j][1], output, strlen(output)) == -1) {
				perror("write");
				free(output);
				deallocate_nodes(node_list);
				deallocate_words(word_list);
				close(fd[j][1]);
				exit(1);
			}

			free(output);
			deallocate_nodes(node_list);
			deallocate_words(word_list);
			close(fd[j][1]);
			free_filenames(filenames, len);
			exit(0);
		} else {
			// close the end of the pipe in the parent process
			// we don't want open
			child_pids[j] = result;
			close(fd[j][1]);
		}
	}

	Node *global_list = NULL;

	// Parent collects each child's message and merges it into one result list.
	for (int j = 0; j < len; j++) {
		char *buffer = read_from_pipe(fd[j][0]);
		close(fd[j][0]);
		merge_child_results(&global_list, buffer);
		free(buffer);
	}

	// Wait for every child so the parent does not leave zombie processes behind.
	for (int j = 0; j < len; j++) {
		int status;
		if (waitpid(child_pids[j], &status, 0) == -1) {
			perror("waitpid");
			deallocate_nodes(global_list);
			free_filenames(filenames, len);
			exit(1);
		}
		if (!WIFEXITED(status) || WEXITSTATUS(status) != 0) {
			fprintf(stderr, "Child %d exited abnormally\n", child_pids[j]);
			deallocate_nodes(global_list);
			free_filenames(filenames, len);
			exit(1);
		}
	}

	// Sorting starts here
	int num_words_distinct = 0;
	for (Node* curr = global_list; curr != NULL; curr = curr->next) {
		num_words_distinct++;
	}

	Node **node_array = malloc(sizeof(Node *) * num_words_distinct);
	if (node_array == NULL) {
		perror("malloc");
		deallocate_nodes(global_list);
		free_filenames(filenames, len);
		exit(1);
	}

	int i = 0;
	for (Node* curr = global_list; curr != NULL; curr = curr->next) {
		node_array[i] = curr;
		i++;
	}
	
	qsort(node_array, num_words_distinct, sizeof(Node *), sort_func);

	
	for (int i = 0; i < num_words_distinct; i++) {
		printf("%s %d\n", node_array[i]->word, node_array[i]->count);
	}

	/*
	// only the parent gets here, so print the final combined word counts
	for (Node *curr = global_list; curr != NULL; curr = curr->next) {
		printf("%s %d\n", curr->word, curr->count);
	}
	*/

	deallocate_nodes(global_list);
	free_filenames(filenames, len);
	return 0;
}

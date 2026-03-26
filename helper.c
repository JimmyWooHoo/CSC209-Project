#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "node.h"

struct Node {
	char word[80];
	int count;
	struct Node *next;
}


char **read_words(char *filename) {
	
	File *fp;
	char **words;

	fp = fopen(filename, "r");
	if (!fp) {
		perror("fopen");
		exit(1);
	}
	
	int count = 0;
	char str[81];
	// count how many words in total in this file
	while (fscanf(fp, "%80s", str) == 1) {
		count++;
	}
	rewind(fp);

	words = malloc((sizeof(*words) * count) + 1);
	if (words == NULL) {
		perror("malloc");
		exit(1);
	}
	
	int i = 0;
	while (fscanf(fp, "%80s", str) == 1) {
		if(strcspn(str, "\n")) {
			str[strcspn(str, "\n")] = '\0';
		} else { // ensure buffer is null-terminated
			str[80] = '\0';
		}
		words[i] = malloc(strlen(str) + 1);
 		strcpy(words[i], str);
		i++;
	}
	
	words[count] = NULL;
	fclose(fp);
	return words;
}

void deallocate_words(char **words) {
	char **p = words;
	while (*p) {
		free(*p);
		p++;
	}
	free(words);
}


Node *new_node(char *str) {
	Node *new = malloc(sizeof(Node));
	if (new == NULL) {
		return NULL;
	}
	
	new->word == malloc(strlen(str) + 1);
	if (!new->word) {
		free(new);
		return NULL;
	}

	strcpy(new->word, str);
	new->count = 1;
	new->next = NULL;

	return new;
}

void add_word_to_node(Node *n, char *str) {
	if(strcmp(n->word, str) == 0) {
		n->count++;
	}
}

Node *find_node(Family *node_list, char *str) {
	Node *head = node_list;
	while(head != NULL) {
		if(strcmp(head->word, str) == 0) {
			// early exit if the word exists
			return head;
		}
		head = head->next;
	}
	// no such word exists;
	return NULL;
}

Node *generate_node_family(char **word_list) {
	Node *head = NULL;
	char **word = word_list;
	

	while (*word != NULL) {
		Node *found = find_node(head, word);
		if (!found) {
			Node *n = new_node(word);
			if(!head) {
				head = n;
				tail = n;
			} else {
				tail->next = n;
				tail = tail->next;
			}
	
		} else {
			add_word_to_node(found, word);
		}
		word++;
	}
	
	return head;

}

char *convert_node_family(Node *n) {
	int count = 0;
	int numwords = 0;
	Node *curr = n;
	while (curr != NULL) {
		count = count + strlen(curr->word);
		numwords++;
		curr = curr->next;
	}
	
	char *str = malloc(count + numwords);
	strcpy(n->word, str);
	numwords--;
	n = n->next;
	while(n != NULL) {
		strncat(str, n->word, sizeof(str) - strlen(str) - 1);
		numwords--;
		if (numwords > 0) {
			strcat(str, " ");
		}
		n = n->next;
	}
	
	str[strlen(str) + 1] = '\0';
	return str;
}

void deallocate_nodes(Node *node_list) {
	while (node_list) {
		Node *next = node_list->next;
		free(node_list->word);
		free(node_list);
		node_list = next;	
	}
}



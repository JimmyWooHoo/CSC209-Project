#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "node.h"

char **read_words(const char *filename) {
	FILE *fp = fopen(filename, "r");
	if (fp == NULL) {
		perror("fopen");
		exit(1);
	}

	int count = 0;
	char str[81];
	while (fscanf(fp, "%80s", str) == 1) {
		count++;
	}
	rewind(fp);

	char **words = malloc(sizeof(*words) * (count + 1));
	if (words == NULL) {
		perror("malloc");
		fclose(fp);
		exit(1);
	}

	int i = 0;
	while (fscanf(fp, "%80s", str) == 1) {
		words[i] = malloc(strlen(str) + 1);
		if (words[i] == NULL) {
			perror("malloc");
			fclose(fp);
			deallocate_words(words);
			exit(1);
		}
		strcpy(words[i], str);
		i++;
	}

	words[count] = NULL;
	fclose(fp);
	return words;
}

void deallocate_words(char **words) {
	if (words == NULL) {
		return;
	}

	for (char **p = words; *p != NULL; p++) {
		free(*p);
	}
	free(words);
}

Node *new_node(const char *str) {
	Node *new = malloc(sizeof(Node));
	if (new == NULL) {
		return NULL;
	}

	strncpy(new->word, str, sizeof(new->word) - 1);
	new->word[sizeof(new->word) - 1] = '\0';
	new->count = 1;
	new->next = NULL;
	return new;
}

Node *find_node(Node *node_list, const char *str) {
	Node *curr = node_list;
	while (curr != NULL) {
		if (strcmp(curr->word, str) == 0) {
			return curr;
		}
		curr = curr->next;
	}
	return NULL;
}

Node *generate_node_family(char **word_list) {
	Node *head = NULL;
	Node *tail = NULL;

	for (char **word = word_list; *word != NULL; word++) {
		Node *found = find_node(head, *word);
		if (found != NULL) {
			found->count++;
			continue;
		}

		Node *new = new_node(*word);
		if (new == NULL) {
			perror("malloc");
			deallocate_nodes(head);
			return NULL;
		}

		if (head == NULL) {
			head = new;
			tail = new;
		} else {
			tail->next = new;
			tail = new;
		}
	}

	return head;
}

void add_count_to_node(Node *node_list, const char *str, int count) {
	Node *found = find_node(node_list, str);
	if (found != NULL) {
		found->count += count;
	}
}

char *convert_node_family(Node *node_list) {
	int total_len = 1;
	for (Node *curr = node_list; curr != NULL; curr = curr->next) {
		total_len += snprintf(NULL, 0, "%s %d\n", curr->word, curr->count);
	}

	char *str = malloc(total_len);
	if (str == NULL) {
		return NULL;
	}

	str[0] = '\0';
	for (Node *curr = node_list; curr != NULL; curr = curr->next) {
		char line[128];
		snprintf(line, sizeof(line), "%s %d\n", curr->word, curr->count);
		strcat(str, line);
	}

	return str;
}

void deallocate_nodes(Node *node_list) {
	while (node_list != NULL) {
		Node *next = node_list->next;
		free(node_list);
		node_list = next;
	}
}

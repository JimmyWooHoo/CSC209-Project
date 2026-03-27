#ifndef NODE_H
#define NODE_H

typedef struct node {
	char word[80];
	int count;
	struct node *next;
} Node;

char **read_words(const char *filename);
void deallocate_words(char **words);

Node *new_node(const char *str);
Node *find_node(Node *node_list, const char *str);
Node *generate_node_family(char **word_list);
void add_count_to_node(Node *node_list, const char *str, int count);
char *convert_node_family(Node *node_list);
void deallocate_nodes(Node *node_list);

#endif

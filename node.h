#ifndef NODE_H
#define NODE_H

struct node {
	char word[80];
	int count;
	struct Node *next;
};
typedef struct node Node;

char **read_words(char *filename);
void deallocate_words(char **words);
Node *new_node(char *str);
void add_word_to_node(Node *n, char *str);
Node *find_node(Family *node_list, char *str);
Node *generate_node_family(char **word_list);
char *convert_node_family(Node *n);
void deallocate_nodes(Node *node_list);

#endif

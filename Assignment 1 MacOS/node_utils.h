
#define BOOL int
#define TRUE 1
#define FALSE 0


//This is the NODE type definition.
//The field question_or_animal contains
//either an animal name if the node is a leaf
//or a question used to descend into the tree.
//The left child represents the node to descend
//to if the answer to the question is "yes", the
//right child represents the node to descend to
//if the answer is "no".

typedef struct node {
  char question_or_animal[200];
  struct node *left;
  struct node *right;
} NODE;


//This reads a line from the standard input.
//It returns true if a line was actually read.
//It returns false if end-of-file was encountered
//before any data could be read.

BOOL read_line(char *p);


//Recursively performs a pre-order traversal of the
//tree starting at node p, printing the question_or_animal field to
//the file specified by the file pointer.

void write_tree(NODE *p, FILE *f);


//Reads the file specified by the file pointer and 
//creates a tree based on the contents of the file.
//Returns a pointer to the root node of the tree.

NODE *read_tree(FILE *f);




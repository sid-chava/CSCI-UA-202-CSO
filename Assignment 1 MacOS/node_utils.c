#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "node_utils.h"

#define BOOL int
#define TRUE 1
#define FALSE 0


//This reads a line from the standard input.
//It returns true if a line was actually read.
//It returns false if end-of-file was encountered
//before any data could be read.

BOOL read_line(char *p)
{
  char c = getchar();
  if (c == EOF)
    return FALSE;
  while ((c == '\n') || (c == ' ') || (c == '\t')) //ignore leading whitespace
    c = getchar();
  while (c != '\n') {
    *p++ = c;
    c = getchar();
  }
  *p = 0;
  return TRUE;
}


//Recursively performs a pre-order traversal of the
//tree starting at node p, printing the question_or_animal field to
//the file specified by the file pointer.

void write_tree(NODE *p, FILE *f)
{
  if (p == NULL)
    fprintf(f,"NULL\n");
  else {
    fprintf(f,"%s\n", p->question_or_animal);
    write_tree(p->left,f);
    write_tree(p->right,f);
  }
}

//Reads the file specified by the file pointer and 
//creates a tree based on the contents of the file.
//Returns a pointer to the root node of the tree.

NODE *read_tree(FILE *f)
{
  char s[200];
  int i;
  NODE *n;
  char c;

  //attempt to read the first character of the line
  c = getc(f);

  //if end-of-file has been reached, then it means
  //that the input wasn't structured correctly.
  if (c == EOF) {
    printf("Error: Wrong number of entries in file\n");
    exit(1);
  }

  //otherwise (not EOF), read characters and
  //insert them into s.
  i = 0;
  for(i=0; (c != EOF) && (c != '\n'); i++) {
    s[i] = c;
    c = getc(f);
  }
  //terminate s with a 0 (null character)
  s[i] = 0;

  //if string just read was the "NULL", then
  //return the NULL pointer.

  if(!strcmp(s,"NULL"))
     return NULL;

  //otherwise, allocate a new node, copy
  //s into the question_or_animal field,
  //and call read_tree twice to
  //read in the left and right subtrees, 
  //respectively. Node this only works because
  //the tree must be a strictly binary tree.
  n = (NODE *) malloc(sizeof(NODE));
  strcpy(n->question_or_animal, s);
  n->left = read_tree(f);
  n->right = read_tree(f);
  return n;
}


#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "node_utils.h"

#define BOOL int
#define TRUE 1
#define FALSE 0

NODE *root = NULL;



// Returns true if user types "yes" or "y" (upper or lower case)
// and returns false if user types "no" or "n". Keeps
// prompting otherwise.

BOOL yes_response()
{
  char response[10];
  while (TRUE)
  {
    scanf("%s", response);
    if (strcasecmp(response, "yes") == 0 || strcasecmp(response, "y") == 0)
      return TRUE;
    else if (strcasecmp(response, "no") == 0 || strcasecmp(response, "n") == 0)
      return FALSE;
    else
      printf("Please answer yes or no: ");
    /* code */
  }
  


  //Fill in the code 
  //Hint: You might consider using the C library function strcasecmp()

}

// This procedure creates a new NODE and copies
// the contents of string s into the 
// question_or_animal field.  It also initializes
// the left and right fields to NULL.
// It should return a pointer to the new node

NODE *new_node(char *s)
{
  NODE *new_node = malloc(sizeof(NODE));
  if (new_node == NULL)
  {
    exit(1);
  }
  
  strcpy(new_node->question_or_animal, s);
  new_node->left = NULL;
  new_node->right = NULL;
  return new_node;


  //Fill in the code
}

// This is the procedure that performs the guessing.
// If the animal is not correctly guessed, it prompts
// the user for the name of the animal and inserts a
// new node for that animal into the tree.

void guess_animal()
{
  NODE *current = root;
  NODE *parent = NULL;
  BOOL direction; // LEFT or RIGHT, represented as BOOL TRUE or FALSE
  const BOOL LEFT = TRUE;
  const BOOL RIGHT = FALSE;

  if (root == NULL) {
    char animal[50];
    printf("What animal were you thinking of? >");
    scanf("%s", animal);
    root = new_node(animal);
    return;
  }

  while (current) {
    if (current->left != NULL && current->right != NULL) {
      printf("%s ", current->question_or_animal);
      if (yes_response()) {
        parent = current;
        direction = LEFT;
        current = current->left;
      } else {
        parent = current;
        direction = RIGHT;
        current = current->right;
      }
    } else {
      printf("Is it a %s? ", current->question_or_animal);
      if (yes_response()) {
        return;
      } else {
        char animal[50];
        printf("What animal were you thinking of? >");
        scanf("%s", animal);
        NODE *new_animal = new_node(animal);
        NODE *new_question = new_node(current->question_or_animal);
        
        printf("What is a yes/no question that distinguishes a %s from a %s? > ", animal, current->question_or_animal);
        char question[200];
        getchar();  // Flush the newline character from the buffer
        fgets(question, sizeof(question), stdin);
        question[strcspn(question, "\n")] = '\0';  // Remove the newline character

        NODE *new_current = new_node(question);
        new_current->left = new_animal;
        new_current->right = new_question;

        if (parent) {
          if (direction == LEFT) {
            parent->left = new_current;
          } else {
            parent->right = new_current;
          }
        } else {
          root = new_current;
        }

        free(current); // Assumes the struct itself was dynamically allocated
        return;
      }
    }
  }
}



//This code is complete. Just add comments where indicated.

int main()
{ int i;
  BOOL done = FALSE;

  //insert comment here
  FILE *datafile = fopen("data.dat", "r"); 

  if (datafile == NULL) {
    printf("data.dat not found\n");
    exit(1);
  }

  //insert comment here
  FILE *backupfile = fopen("data.dat.bak", "w");

  //insert comment here
  root = read_tree(datafile);

  //call write_tree() to write the initial tree to the
  //backup file (i.e. to backup the tree data)
  write_tree(root,backupfile);

  //close both files (for now)
  fclose(backupfile);
  fclose(datafile);

  printf("Welcome to the animal guessing game (like 20 questions).\n");
  do { 
    printf("\nThink of an animal...\n");
    guess_animal();  //insert comment here
    printf("\nDo you want to play again? (yes/no) >");
  } while (yes_response());  //insert comment here


  //now open the "data.dat" file for writing
  datafile = fopen("data.dat", "w");

  //insert comment
  write_tree(root, datafile);

  //close the data.dat file
  fclose(datafile);

}


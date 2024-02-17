
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "binary_search_tree.h"

#define SIZE 150

EMPLOYEE unsorted_employees[SIZE];

EMPLOYEE sorted_employees[SIZE];

int main(int argc, char *argv[])
{
  // The program must be invoked with two parameters: the name of the
  // input file and the name of the output file.
  
  if (argc != 3) {
    printf("Two parameters required: an input file name and an output file name\n");
    exit(1);
  }

  // Open the input file for reading and create the output file to write to.
  
  FILE *input = fopen(argv[1], "r");
  FILE *output = fopen(argv[2], "w");

  int count = 0;

  char first[NAME_SIZE], last[NAME_SIZE];
  long id, salary;

  // Using fsanf to read from the input file. Since we are providing the
  // input file, we won't defend against buffer overflow attack here.
  // fscanf returns the number of data items actually read, so if it's
  // less than 4, we know we have hit the end of the input file.

  // read the first employee record
  
  int res = fscanf(input, "%s %s %ld %ld", first, last, &id, &salary);

  // We're limited to SIZE (150) employee records, so we'll keep
  // reading employee records until we hit the limit or until
  // there are no more records to read.
  
  while ((count < SIZE) && (res == 4)) {

    // write the first name, last name, id, and salary
    // into the unsorted_employee[] array.
    
    strcpy(unsorted_employees[count].first,first);
    strcpy(unsorted_employees[count].last,last);
    unsorted_employees[count].id = id;
    unsorted_employees[count].salary = salary;

    // read the next employee record, increment count, and
    // continue to the next iteration of the loop.
    
    res = fscanf(input, "%s %s %ld %ld", first, last, &id, &salary);
    count++;
  }

  // This loop calls insert_employee() on each employee
  // record in the unsorted_employees array to insert
  // the record into the binary search tree.
  
  for(int i = 0; i < count; i++) {
    insert_employee(unsorted_employees[i]);
  }


  // In a loop, remove all employees from the binary
  // search tree in alphabetical order, placing the
  // removed employee information into the
  // sorted_employees[] array. That is, 
  // each time through the loop, remove the least
  // (alphabetically speaking) node from the tree.

  // Note: remove_employee returns 0 if there was an
  // employee to remove, otherwise it returns
  // 1 if the tree was empty.

  int removed_count = 0;

  for(int i = 0, result = 0; (!result) && (i < count); i++) {
    result = remove_employee(&sorted_employees[i]);
    if (!result)
      removed_count++;
  }


  // Finally, write out the contents of the sorted_employees
  // array to the output file.

  for(int i = 0; i < removed_count; i++) {
    EMPLOYEE *p = &sorted_employees[i];
    fprintf(output, "%s, %s, %ld, %ld\n", p->last, p->first, p->id, p->salary);
  }
    
}


#define NAME_SIZE 100

typedef struct {
  long id;
  long salary;  
  char first[NAME_SIZE];
  char last[NAME_SIZE];

} EMPLOYEE;

typedef struct node {
  EMPLOYEE person;
  struct node *left;
  struct node *right;
} NODE;

extern NODE *root;

NODE *new_node(EMPLOYEE employee);
void insert_node(NODE *new_n);
void insert_employee(EMPLOYEE employee);

NODE *remove_smallest();
int remove_employee(EMPLOYEE *employee);



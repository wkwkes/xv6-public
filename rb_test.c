#include "rb_test.h"

const int node_size = 29;
char* node_pool;
int node_pool_acc;

char*
kalloc()
{
  return malloc(4096);
}

void
panic(char* str)
{
  printf("panic! : %s\n", str);
  exit(0);
}

char* node_pool;
int node_pool_acc;

int
is_used(char* ptr)
{
  if (ptr[node_size - 1] == 1) {
    return 1;
  } else {
    return 0;
  }
}

struct node*
alloc_node_(int c)
{
  //   printf("alloc (%d, %d)\n", node_pool_acc, c);
  if (node_pool_acc + node_size <= 4096) {
    node_pool_acc += node_size;
    if (is_used(&node_pool[node_pool_acc - node_size]) == 0) {
      node_pool[node_pool_acc - 1] = 1;
      // printf("ret!\n");
      return (struct node*)&node_pool[node_pool_acc - node_size];
    } else {
      return alloc_node_(c);
    }
  } else {
    if (c == 1) {
      panic("out of memory : alloc_node!");
    } else {
      node_pool_acc = 0;
      return alloc_node_(1);
    }
  }
}

struct node*
alloc_node()
{
  return alloc_node_(0);
}

void
free_node(struct node* nd)
{
  char* ptr = (char*)nd;
  ptr[node_size - 1] = 0;
}

void
dump_byte(struct node* nd)
{
  int i, j;
  for (i = 0; i < 29; i++) {
    if (i % 4 == 0) {
      printf("--------\n");
    }
    char c = ((char*)nd)[i];
    for (j = 0; j < 8; j++) {
      printf("%d", c & 1);
      c >>= 1;
    }
    printf("\n");
  }
}

void
new_proc(int pri, int pid)
{
  struct node* nd = alloc_node();
  init_node(nd, pri, RED, pid - 1, 0, NULL, NULL, NULL);
  add_proc(nd);
  dump_nodes(groot, 0);
}

int
get_length(struct node* nd, int i)
{
  if (nd == NULL_) {
    return i + 1;
  }
  if (nd->color == BLACK) {
    return get_length(nd->left, i + 1);
  } else {
    return get_length(nd->left, i);
  }
}

int
real_rb_tree(struct node* nd, int i, int len, int parent_pri)
{
  if (nd == NULL_) {
    if (i + 1 == len) {
      return 1;
    } else {
      return 0;
    }
  }
  if (i > len) {
    return 0;
  }

  if (nd->dir) { // right
    if (nd->pri < parent_pri) {
      printf("%d, %d\n", parent_pri, nd->pri);
      return 0;
    }
  } else { // left
    if (nd->pri >= parent_pri) {
      printf("%d, %d\n", parent_pri, nd->pri);
      return 0;
    }
  }

  int j = i;
  if (nd->color == BLACK) {
    j++;
  }
  return (real_rb_tree(nd->left, j, len, nd->pri)) &&
         (real_rb_tree(nd->right, j, len, nd->pri));
}

void
rb_check(struct node* nd)
{
  if (nd == NULL_) {
    printf("this is NULL\n");
    return;
  }
  int len = get_length(nd->left, 0);
  if (real_rb_tree(nd->left, 0, len, nd->pri) &&
      real_rb_tree(nd->right, 0, len, nd->pri)) {
    printf("this is a real rb tree\n");
  } else {
    printf("NO!\n");
  }
}

int
main()
{
  /*************** initialization *********************/
  if ((node_pool = kalloc()) == 0) {
    panic("pinit: out of memory(node_pool)?");
  }
  int i;
  for (i = 0; i < 4096; i++)
    node_pool[i] = 0;
  node_pool_acc = 0;
  /****************************************************/

  //   new_proc(1000, 1);
  //   dump_nodes(groot, 0);
  //   new_proc(900, 2);
  //   dump_nodes(groot, 0);
  //   new_proc(800, 3);
  //   dump_nodes(groot, 0);
  //   new_proc(700, 4);
  //   dump_nodes(groot, 0);
  //   new_proc(600, 5);
  //   dump_nodes(groot, 0);
  printf("%d\n", abs(random()));

  for (int i = 0; i < 141; i++) {
    int j = abs(random()) % 1000;
    printf("%d, %d\n", j, i);
    new_proc(j, i);
  }

  printf("\n\n");
//   struct node* nd = get_proc();
//   free_node(nd);
//   new_proc(5000, 1000);
  //   new_proc(5000, 1000);

  rb_check(groot);
  
  for (int i = 0; i < 141; i++) {
    struct node* nd = get_proc();
    if (nd == NULL) {
        printf("NULL?????\n");
        break;
    }
    printf("%d\n", nd->pri);
    free_node(nd);
  }
  
  //   struct node* nd1 = alloc_node();
  //   init_node(nd1, 1000, RED, 1, 0, NULL, NULL, NULL);
  //   struct node* nd2 = alloc_node();
  //   struct node* nd2 = malloc(node_size - 1);
  //   init_node(nd2, 1000, RED, 2, 0, NULL, NULL, NULL);
  //   struct node* nd3 = alloc_node();
  //   struct node* nd3 = malloc(node_size - 1);
  //   init_node(nd3, 1000, RED, 3, 0, NULL, NULL, NULL);

  //   add_proc(nd1);
  //   dump_nodes(groot, 0);
  //   add_proc(nd2);
  //   dump_nodes(groot, 0);
  //   add_proc(nd3);
  //   dump_nodes(groot, 0)P;

  //   dump_byte(nd1);

  //   i = 0;
  //   while (i++ < 140) {
  //     alloc_node(0);
  //   }
  //   alloc_node(0);
  //   free_node(node_pool + node_pool_acc - node_size);
  //   alloc_node(0);

}
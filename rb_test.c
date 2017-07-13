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
  init_node(nd, pri, RED, pid - 1, 0, NULL_, NULL_, NULL_);
  add_proc(nd);
  // dump_nodes(groot, 0);
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
  int len = 20;

  for (int i = 0; i < len; i++) {
    int j = abs(random()) % 1000;
    printf("%d, %d\n", j, i);
    new_proc(j, i+1);
    // dump_nodes(groot, 0);
    // printf("\n");
  }

  rb_check(groot);
  // dump_nodes(groot, 0);
  printf("\n\n");

  // printf("ko re -> %d, %d\n", groot->pri, groot->proc_index);
  struct node *md = get_node(groot, groot->pri, groot->proc_index);
  printf("get_node : \n");
  dump_node(md, 0);
  // dump_nodes(groot, 0);

  for (int i = 0; i < len; i++) {
    rb_check(groot);
    // dump_nodes(groot, 0);
    struct node* nd = get_proc();
    if (nd == NULL_) {
        printf("NULL %d\n", i);
        break;
    }
    // printf("%d\n\n", nd->pri);
    printf("%d, %d\n", nd->pri, i);
    free_node(nd);
    // dump_nodes(groot, 0);
  }
  // dump_nodes(groot, 0);
  return 1;
}

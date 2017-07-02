// gcc  -O2 -Wall -MD -ggdb -m32 -Werror -fno-omit-frame-pointer rb.c
#include <stdio.h>
#include <stdlib.h>

#define size 64

enum Color
{
  RED,
  BLACK
};

struct proc
{
  int pri;
  int data;
};

struct proc procs[size];

struct node
{
  int pri;
  enum Color color; // red : 0, black : 1
  int proc_index;
  int dir; // left : 0, right : 1
  struct node* parent;
  struct node* left;
  struct node* right;
};

struct node* groot;
void
groot_mod(struct node* nd)
{
  while (nd->parent)
    nd = nd->parent;
  groot = nd;
}

void
node_init(struct node* nd,
          int pri,
          enum Color color,
          int proc_index,
          int dir,
          struct node* parent,
          struct node* left,
          struct node* right)
{
  nd->pri = pri;
  nd->color = color;
  nd->proc_index = proc_index;
  nd->dir = dir;
  nd->parent = parent;
  nd->left = left;
  nd->right = right;
}

void
dump_node(struct node* nd, int indent)
{
  for (int i = 0; i < indent; i++)
    printf(" ");
  printf("+----------\n");
  for (int i = 0; i < indent; i++)
    printf(" ");
  printf("|pri:%d\n", nd->pri);

  for (int i = 0; i < indent; i++)
    printf(" ");

  if (nd->color == RED) {
    printf("|color:RED\n");
  } else {
    printf("|color:BLACK\n");
  }

  for (int i = 0; i < indent; i++)
    printf(" ");
  printf("|proc_index:%d\n", nd->proc_index);

  for (int i = 0; i < indent; i++)
    printf(" ");
  if (nd->dir == 1) {
    printf("|dir:right\n");
  } else {
    printf("|dir:left\n");
  }
  if (nd->parent != NULL) {
    for (int i = 0; i < indent; i++)
      printf(" ");
    printf("|parent_id:%d\n", nd->parent->proc_index);
  }
  for (int i = 0; i < indent; i++)
    printf(" ");
  printf("+----------\n");
}

void
dump_nodes(struct node* root, int indent)
{
  dump_node(root, indent);

  if (root->left != NULL) {
    dump_nodes(root->left, indent + 5);
  }

  if (root->right != NULL) {
    dump_nodes(root->right, indent + 5);
  }
}

void
swap(struct node* nd1, struct node* nd2)
{ // sallow swap
  // tmps
  int nd_pri = nd1->pri;
  enum Color nd_color = nd1->color;
  int nd_proc_index = nd1->proc_index;
  int nd_dir = nd1->dir;

  // nd1 => nd2
  nd1->pri = nd2->pri;
  nd1->color = nd2->color;
  nd1->proc_index = nd2->proc_index;
  nd1->dir = nd2->dir;

  // nd2 => nd
  nd2->pri = nd_pri;
  nd2->color = nd_color;
  nd2->proc_index = nd_proc_index;
  nd2->dir = nd_dir;
}

void
insert_helper(struct node* root, struct node* nd)
{
  if (nd->pri < root->pri) {
    nd->dir = 0;
    if (root->left == NULL) {
      root->left = nd;
      nd->parent = root;
      groot_mod(root);
    } else {
      insert_helper(root->left, nd);
    }
  } else {
    nd->dir = 1;
    if (root->right == NULL) {
      root->right = nd;
      nd->parent = root;
      groot_mod(root);
    } else {
      insert_helper(root->right, nd);
    }
  }
}

void
rotate_right(struct node* c, struct node* p)
{ // c is the left child of p

  struct node* sub = c->right;
  c->right = p;
  c->parent = p->parent;

  if (p->parent) {
    if (p->dir == 0) {
      p->parent->left = c;
    } else {
      p->parent->right = c;
    }
  }
  p->parent = c;
  p->left = sub;

  if (sub)
    sub->dir = 0;
  c->dir = p->dir;
  p->dir = 1;
}

void
rotate_left(struct node* c, struct node* p)
{ // c is the right child of p
  struct node* sub = c->left;
  c->left = p;
  c->parent = p->parent;
  if (p->parent) {
    if (p->dir == 0) {
      p->parent->left = c;
    } else {
      p->parent->right = c;
    }
  }
  p->parent = c;
  p->right = sub;

  if (sub)
    sub->dir = 1;
  c->dir = p->dir;
  p->dir = 0;
}

void
insert(struct node* root, struct node* nd)
{
  nd->color = RED;

  insert_helper(root, nd);

  // printf("\n");
  // dump_nodes(groot, 10);
  // printf("\n");

  while (1) {
    if (nd->parent == NULL) {
      nd->color = BLACK;
      break;
    }

    struct node* par = nd->parent;
    if (par->color == BLACK) {
      break;
    }
    struct node* gp = par->parent;
    struct node* uncle;
    if (par->dir) { // right
      uncle = gp->left;
    } else {
      uncle = gp->right;
    }

    enum Color uncle_color;
    if (uncle == NULL) {
      uncle_color = BLACK;
    } else {
      uncle_color = uncle->color;
    }

    if (uncle_color == RED) {
      par->color = BLACK;
      uncle->color = BLACK;

      gp->color = RED;

      // swap(nd, gp);
      nd = gp;
      continue;
    } else {                               // rotation
      if (par->dir == 0 && nd->dir == 0) { // left-left
        par->color = BLACK;
        gp->color = RED;
        rotate_right(par, gp);
      } else if (par->dir == 0 && nd->dir == 1) { // left-right
        nd->color = BLACK;
        gp->color = RED;
        rotate_left(nd, par);
        rotate_right(nd, gp);
      } else if (par->dir == 1 && nd->dir == 0) { // right-left
        nd->color = BLACK;
        gp->color = RED;
        rotate_right(nd, par);
        rotate_left(nd, gp);
      } else if (par->dir == 1 && nd->dir == 1) { // right-right
        par->color = BLACK;
        gp->color = RED;
        rotate_left(nd, par);
      }
      break;
    }
  }
  // find root
  groot_mod(nd);
}

void delete (struct node* root, int proc_index)
{
  struct node* u = NULL; // child that replace v
  struct node* v = NULL; // to delete
  struct node* root_ = root;
  while (u == NULL && v == NULL) {
    if (root_ == NULL) {
      break;
    }
    printf("here!!\n");
    if (root_->proc_index == proc_index) { // delete root
      printf("here!!\n");
      if (root_->left != NULL && root_->right != NULL) {
        struct node* min_node = root_->right;
        while (min_node->left != NULL) {
          min_node = min_node->left;
        }
        printf("here!!\n");
        swap(min_node, root_);
        root_ = min_node;
        proc_index = min_node->proc_index;
      } else if (root_->left != NULL) {
        u = root_->left;
        v = root_;
      } else if (root_->right != NULL) {
        u = root_->right;
        v = root_;
      } else {
        v = root_;
      }
    } else if (proc_index < root_->proc_index) {
      printf("here!ll(%d, %d)\n", proc_index, root_->proc_index);
      root_ = root_->left;
    } else { // proc_index > root_->proc_index
      printf("here!rr(%d, %d)\n", proc_index, root_->proc_index);
      root_ = root_->right;
    }
  }

  if (u) {
    printf("\nu\n");
    dump_node(u, 10);
    groot_mod(u);
  } else {
    printf("u is NULL\n");
  }
  if (v) {
    printf("\nv\n");
    dump_node(v, 10);
    groot_mod(v);
  } else {
    printf("v is NULL\n");
  }
  dump_nodes(groot, 0);
}

int
main()
{
  struct node* nd = malloc(sizeof(struct node));
  struct node* l = malloc(sizeof(struct node));
  struct node* r = malloc(sizeof(struct node));
  struct node* ll = malloc(sizeof(struct node));
  struct node* ins = malloc(sizeof(struct node));

  node_init(nd, 1000, BLACK, 0, 0, NULL, NULL, NULL);
  node_init(l, 900, RED, 1, 0, NULL, NULL, NULL);
  node_init(r, 800, RED, 2, 1, NULL, NULL, NULL);
  node_init(ll, 700, RED, 3, 0, NULL, NULL, NULL);
  node_init(ins, 850, RED, 4, 0, NULL, NULL, NULL);

  // node_init(nd, 1000, BLACK, 0, 0, NULL, l, r);
  // node_init(l, 900, BLACK, 1, 0, nd, ll, NULL);
  // node_init(r, 800, BLACK, 2, 1, nd, NULL, NULL);
  // node_init(ll, 700, RED, 3, 0, l, NULL, NULL);
  // node_init(ins, 850, RED, 4, 0, NULL, NULL, NULL);

  dump_nodes(nd, 0);
  groot = nd;
  printf("\n\n\n");
  insert(groot, l);
  dump_nodes(groot, 0);

  printf("\n\n\n");
  insert(groot, r);
  dump_nodes(groot, 0);

  printf("\n\n\n");
  insert(groot, ll);
  dump_nodes(groot, 0);

  printf("\n\n\n");
  insert(groot, ins);
  dump_nodes(groot, 0);

  delete (groot, 2);

  free(nd);
  free(l);
  free(r);
  free(ll);
  free(ins);
  return 1;
}
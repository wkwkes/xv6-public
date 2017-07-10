
#include <stdio.h>
#include <stdlib.h>

#define NULL_ ((void*)0)

enum Color
{
  RED,
  BLACK
};

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
init_node(struct node* nd,
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
  int i;
  for (i = 0; i < indent; i++)
    printf(" ");
  printf("+----------\n");
  for (i = 0; i < indent; i++)
    printf(" ");
  printf("|pri:%d\n", nd->pri);

  for (i = 0; i < indent; i++)
    printf(" ");

  if (nd->color == RED) {
    printf("|color:RED\n");
  } else {
    printf("|color:BLACK\n");
  }

  for (i = 0; i < indent; i++)
    printf(" ");
  printf("|proc_index:%d\n", nd->proc_index);

  for (i = 0; i < indent; i++)
    printf(" ");
  if (nd->dir == 1) {
    printf("|dir:right\n");
  } else {
    printf("|dir:left\n");
  }

  for (i = 0; i < indent; i++)
    printf(" ");
  if (nd->parent != NULL_) {
    printf("|parent_id:%d\n", nd->parent->proc_index);
  } else {
    printf("|parent_id:NULL_\n");
  }

  for (i = 0; i < indent; i++)
    printf(" ");
  if (nd->left == NULL_) {
    printf("|left:NULL_\n");
  } else {
    printf("|left:%d\n", nd->left->proc_index);
  }

  for (i = 0; i < indent; i++)
    printf(" ");
  if (nd->right == NULL_) {
    printf("|right:NULL_\n");
  } else {
    printf("|right:%d\n", nd->right->proc_index);
  }
  for (i = 0; i < indent; i++)
    printf(" ");
  printf("+----------\n");
}

void
dump_nodes(struct node* root, int indent)
{
  if (root == NULL_) 
    return;
    
  dump_node(root, indent);

  if (root->left != NULL_) {
    dump_nodes(root->left, indent + 5);
  }

  if (root->right != NULL_) {
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
  if (root == NULL_) {
    groot = nd;
    return;
  }
  if (nd->pri < root->pri) {
    nd->dir = 0;
    if (root->left == NULL_) {
      root->left = nd;
      nd->parent = root;
      groot_mod(root);
    } else {
      insert_helper(root->left, nd);
    }
  } else {
    nd->dir = 1;
    if (root->right == NULL_) {
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
  
  // TODO
  nd->left = nd->right = NULL_;

  insert_helper(root, nd);

  while (1) {
    if (nd->parent == NULL_) {
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
    if (uncle == NULL_) {
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
        rotate_left(par, gp);
      }
      break;
    }
  }
  // find root
  groot_mod(nd);
}

void
reduce(struct node* p)
{
  if (p == NULL_) {
    return;
  }
  struct node* s = p->right; // s cannot be NULL_
  if (s->color == BLACK) {
    if (s->left == NULL_ && s->right == NULL_) {
      s->color = RED;
      if (p->color == RED) {
        p->color = BLACK;
      } else {
        reduce(p->parent);
      }
    } else if (s->right != NULL_) { // s->right must be RED
      rotate_left(s, p);
      s->right->color = BLACK;
    } else if (s->left != NULL_) {
      struct node* r = s->left;
      rotate_right(r, s);
      rotate_left(r, p);
    }
  } else {
    rotate_left(s, p);
    reduce(p);
  }
}

struct node*
get_min(struct node* root)
{
  struct node* v = root; // min node
  if (v == NULL_) {
    return NULL_;
  }
  while (v->left != NULL_) {
    v = v->left;
  }
  if (v->parent == NULL_) {
    // v->left must be NULL_
    groot = v->right;
    if (v->right != NULL_) {
      v->right->parent = NULL_;
      v->right->color = BLACK;
    }
    return v;
  }
  if (v->color == RED) {
    // when v is leftmost and RED, v has no children because
    //   * v doesn't have a left child since it is leftmost
    //   * v doesn't have a RED right child since v is RED
    //   * v doesn't have a BLACK right child since the
    //     right child's leaf's path has much BLACK nodes
    //     than left.
    // that is v->left == NULL_ and v->right == NULL_.
    v->parent->left = NULL_;
  } else {
    if (v->right != NULL_) {
      // v->right must be RED
      v->right->dir = 0;
      v->right->color = BLACK;
      v->right->parent = v->parent;
      v->parent->left = v->right;
    } else { // v->right is leaf (with two NULL_s)
      v->parent->left = NULL_;
      reduce(v->parent);
    }
  }
  return v;
}

void
add_proc(struct node* nd)
{
  insert(groot, nd);
}

struct node*
get_proc()
{
  return get_min(groot);
}
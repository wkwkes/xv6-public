
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
  // for (i = 0; i < indent; i++)
  //   printf(" ");
  // printf("+----------\n");
  for (i = 0; i < indent; i++)
    printf(" ");
  if (nd->color == RED)
    printf("|pri:%d RED", nd->pri);
  if (nd->color == BLACK)
    printf("|pri:%d BLK", nd->pri);

  if (nd->parent) {
    printf(" -> %d\n", nd->parent->pri);
  } else {
    printf("\n");
  }
  // for (i = 0; i < indent; i++)
  //   printf(" ");

  // if (nd->color == RED) {
  //   printf("|color:RED\n");
  // } else {
  //   printf("|color:BLACK\n");
  // }

  // for (i = 0; i < indent; i++)
  //   printf(" ");
  // printf("|proc_index:%d\n", nd->proc_index);

  // for (i = 0; i < indent; i++)
  //   printf(" ");
  // if (nd->dir == 1) {
  //   printf("|dir:right\n");
  // } else {
  //   printf("|dir:left\n");
  // }

  // for (i = 0; i < indent; i++)
  //   printf(" ");
  // if (nd->parent != NULL_) {
  //   printf("|parent_id:%d\n", nd->parent->proc_index);
  // } else {
  //   printf("|parent_id:NULL_\n");
  // }

  // for (i = 0; i < indent; i++)
  //   printf(" ");
  // if (nd->left == NULL_) {
  //   printf("|left:NULL_\n");
  // } else {
  //   printf("|left:%d\n", nd->left->proc_index);
  // }

  // for (i = 0; i < indent; i++)
  //   printf(" ");
  // if (nd->right == NULL_) {
  //   printf("|right:NULL_\n");
  // } else {
  //   printf("|right:%d\n", nd->right->proc_index);
  // }
  // for (i = 0; i < indent; i++)
  //   printf(" ");
  // printf("+----------\n");
}

void
dump_nodes(struct node* root, int indent)
{
  if (root == NULL_) {
    printf("NULL tree\n");
    return;
  }

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
  // printf("pohe\n");
  if (root == NULL_) {
    groot = nd;
    return;
  }
  if (nd->pri < root->pri) {
    nd->dir = 0;
    // printf("maji?\n");
    if (root->left == NULL_) {
      // printf("maji 2?\n");
      root->left = nd;
      nd->parent = root;
      groot_mod(root);
    } else {
      // printf("maji 3?\n");
      insert_helper(root->left, nd);
    }
  } else {
    nd->dir = 1;
    // printf("maji de?\n");
    if (root->right == NULL_) {
      // printf("maji de 2?\n");
      root->right = nd;
      nd->parent = root;
      // printf("koko\n");
      groot_mod(root);
    } else {
      if (root->right == NULL_) {
        // printf("komatta!!!\n");
      }
      // printf("maji de 3?\n");
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

  if (sub) {
    sub->dir = 0;
    sub->parent = p;
  }
  c->dir = p->dir;
  p->dir = 1;
  groot_mod(c);
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

  if (sub) {
    sub->dir = 1;
    sub->parent = p;
  }
  c->dir = p->dir;
  p->dir = 0;
  groot_mod(c);
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
      nd = gp;
      continue;
    } else {                               // rotation
      if (par->dir == 0 && nd->dir == 0) { // left-left
        rotate_right(par, gp);
        par->color = BLACK;
        gp->color = RED;
      } else if (par->dir == 0 && nd->dir == 1) { // left-right
        rotate_left(nd, par);
        rotate_right(nd, gp);
        nd->color = BLACK;
        gp->color = RED;
      } else if (par->dir == 1 && nd->dir == 0) { // right-left
        rotate_right(nd, par);
        rotate_left(nd, gp);
        nd->color = BLACK;
        gp->color = RED;
      } else if (par->dir == 1 && nd->dir == 1) { // right-right
        rotate_left(par, gp);
        par->color = BLACK;
        gp->color = RED;
      }
      break;
    }
  }
  // find root
  groot_mod(nd);
}

void
reduce(struct node* par, int dir)
{
  // printf("-----\n");
  // if (par) {
  //   printf("reduce(%d, %d)\n", par->pri, dir);
  // } else {
  //   printf("par is NULL\n");
  // }
  // dump_nodes(groot, 0);
  // printf("-----\n");
  if (par == NULL_) {
    return;
  }
  struct node* p = par;
  struct node* s;
  if (dir == 0) { // left
    s = p->right;
  } else {
    s = p->left;
  }
  enum Color s_color;
  if (s == NULL_) {
    return;
  } else {
    s_color = s->color;
  }
  if (s_color == BLACK) {
    struct node* r;
    enum Color r_color;
    int r_is_red = 0;
    if (s->dir == 0) { // s is left
      if (s->left != NULL_ && s->left->color == RED) {
        r_is_red = 1;
        r = s->left;
        r->color = BLACK;
        if (p->color == RED) {
          s->color = RED;
          p->color = BLACK;
        }
        rotate_right(s, p);
      } else if (s->right != NULL_) {
        if (s->right->color == RED) { // left-right
          r_is_red = 1;
          r = s->right;
          r->color = BLACK;
          p->color = BLACK;
          s->color = RED;
          rotate_right(r, s);
          reduce(p, 0);
        }
      }
    } else { // s is right
      if (s->right != NULL_ && s->right->color == RED) {
        r_is_red = 1;
        r = s->right;
        r->color = BLACK;
        if (p->color == RED) {
          s->color = RED;
          p->color = BLACK;
        }
        rotate_left(s, p);
      } else if (s->left != NULL_) {
        if (s->left->color == RED) { // right-left
          r_is_red = 1;
          r = s->left;
          r->color = BLACK;
          s->color = RED;
          rotate_right(r, s);
          reduce(p, 0);
        }
      }
    }
    if (r_is_red == 0) {
      s->color = RED;
      if (p->color == RED) {
        p->color = BLACK;
      } else {
        reduce(p->parent, p->dir);
      }
    }
  } else { // s_color == RED
    if (s->dir == 0) { // left
      s->color = BLACK;
      p->color = RED;
      rotate_right(s, p);
      reduce(p, 1);
    } else { // right
      s->color = BLACK;
      p->color = RED;
      rotate_left(s, p);
      reduce(p, 0);
    }
  }
}

// v is the node to delete
void delete (struct node* root, struct node* v)
{
  if (v == NULL_) {
    printf("delete : v is NULL\n");
    return;
  }
  if (v == root && v->left == NULL_ && v->right == NULL_) {
    groot = NULL_;
    return;
  } 

  enum Color v_color = v->color;
  if (v->left && v->right) {
    printf("v is a inner node\n");
    return;
  }
  struct node* u;
  if (v->left != NULL_) {
    u = v->left;
  } else if (v->right != NULL_) {
    u = v->right;
  } else { // v is a leaf
    u = NULL_;
  }
  enum Color u_color;
  if (u != NULL_) {
    u_color = u->color;
  } else {
    u_color = BLACK;
  }
  if (v_color == RED || u_color == RED) { // (2)
    // printf("RED pattern\n");
    if (v->parent == NULL_) {
      groot = u;
      return;
    }
    if (v->dir == 0) {
      v->parent->left = u;
    } else {
      v->parent->right = u;
    }
    if (u != NULL_) {
      u->parent = v->parent;
      u->dir = v->dir;
      u->color = BLACK;
    }
  } else if (v_color == BLACK &&
             u_color == BLACK) { // (3) in this pattern, u must be NULL
    if (v->parent == NULL_) {
      groot = NULL_;
      return;
    }
    if (v->dir == 0) {
      v->parent->left = NULL_;
    } else {
      v->parent->right = NULL_;
    }
    reduce(v->parent, v->dir);
  }
}

// if root == NULL_, return NULL_
struct node*
get_min(struct node* root)
{
  struct node* v = root;
  if (v == NULL_) {
    return NULL_;
  }
  while (v->left != NULL_) {
    v = v->left;
  }
  delete (root, v);
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
/* Copyright 2022 Julian Ingram
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 * http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "lbtree.h"

static void node_copy(struct lbtree *dst, struct lbtree *src) {
  struct lbtree **src_children = src->children;
  struct lbtree **dst_children = dst->children;
  unsigned int i;
  for (i = 0; i < LBTREE_LUT_SIZE; ++i) {
    dst_children[i] = (src_children[i] == src) ? dst : src_children[i];
  }
  dst->index = src->index;
}

static void children_init(struct lbtree *node) {
  unsigned int i;
  for (i = 0; i < LBTREE_LUT_SIZE; ++i) {
    node->children[i] = node;
  }
}

void lbtree_init(struct lbtree *node) {
  children_init(node);
  lbtree_index_init(&node->index);
}

void lbtree_repl(struct lbtree **tree,
                 unsigned int (*sel_node)(void *node, lbtree_index_t index),
                 struct lbtree *match, struct lbtree *node) {
  struct lbtree **ref = tree;
  while (*ref != match) {
    struct lbtree *parent = *ref;
    ref = &parent->children[sel_node(match, parent->index)];
  };
  node_copy(node, match);
  *ref = node;
  /* continue lookup and replace final ref to replaced node. */
  lbtree_index_t index;
  lbtree_index_t parent_index;
  struct lbtree *parent;
  do {
    parent = *ref;
    parent_index = parent->index;
    ref = &parent->children[sel_node(node, parent_index)];
    index = (*ref)->index;
  } while (lbtree_index_gt(index, parent_index) != 0);
  *ref = node;
}

void lbtree_add(struct lbtree **tree,
                unsigned int (*sel_node)(void *node, lbtree_index_t index),
                struct lbtree *node) {
  struct lbtree *parent = *tree;
  lbtree_index_t index;
  lbtree_index_t parent_index;
  struct lbtree **ref = tree;
  struct lbtree *next_parent = parent;
  unsigned int ref_sel = sel_node(*tree, (*tree)->index);
  do {
    parent_index = next_parent->index;
    if (lbtree_index_gt(parent_index, node->index) != 0) {
      break;
    }
    parent = next_parent;
    ref_sel = sel_node(node, parent_index);
    ref = &parent->children[ref_sel];
    next_parent = *ref;
    index = next_parent->index;
  } while (lbtree_index_gt(index, parent_index) != 0);
  struct lbtree *cut = *ref;
  children_init(node);
  /* add ref to upper tree */
  if (sel_node(cut, parent->index) == ref_sel) {
    node->children[sel_node(cut, node->index)] = cut;
  }
  *ref = node;
}

void *lbtree(struct lbtree *tree,
             unsigned int (*sel_key)(void *key, lbtree_index_t index),
             void *key) {
  if (tree == 0) {
    return 0;
  }
  lbtree_index_t index;
  lbtree_index_t parent_index;
  do {
    parent_index = tree->index;
    tree = tree->children[sel_key(key, parent_index)];
    index = tree->index;
  } while (lbtree_index_gt(index, parent_index) != 0);
  return (struct lbtree *)tree;
}

struct lbtree_leaf_pos
lbtree_leaf_pos(struct lbtree **tree,
                unsigned int (*sel)(void *key, lbtree_index_t index), void *key,
                struct lbtree *parent) {
  lbtree_index_t parent_index;
  unsigned int child_sel;
  struct lbtree **parent_ref;
  struct lbtree *grandparent;
  struct lbtree *leaf;
  struct lbtree **ref = tree;
  do {
    parent_ref = ref;
    grandparent = parent;
    parent = *parent_ref;
    parent_index = parent->index;
    child_sel = sel(key, parent_index);
    ref = &parent->children[child_sel];
    leaf = *ref;
  } while (lbtree_index_gt(leaf->index, parent_index) != 0);
  struct lbtree_leaf_pos pos = {.cut = parent->children[child_sel ^ 1],
                                .leaf = leaf,
                                .parent_ref = parent_ref,
                                .grandparent = grandparent};
  return pos;
}

struct lbtree_branch_pos
lbtree_branch_pos(struct lbtree **tree,
                  unsigned int (*sel)(void *key, lbtree_index_t index),
                  struct lbtree *node, void *key) {
  struct lbtree_branch_pos pos = {.ref = tree, .parent = 0};
  while (*pos.ref != node) {
    pos.parent = *pos.ref;
    pos.ref = &pos.parent->children[sel(key, pos.parent->index)];
  };
  return pos;
}

void lbtree_cut(struct lbtree **branch_ref, struct lbtree_leaf_pos leaf_pos) {
  struct lbtree *leaf_parent = *leaf_pos.parent_ref;
  /* replace free node with its non-key-ref child */
  if (leaf_pos.cut != leaf_pos.leaf) {
    *leaf_pos.parent_ref = leaf_pos.cut;
    if (leaf_parent != leaf_pos.leaf) {
      node_copy(leaf_parent, leaf_pos.leaf);
      /* replace key branch with free node */
      *branch_ref = leaf_parent;
    }
  } else {
    *leaf_pos.parent_ref = leaf_pos.grandparent;
  }
}

void *lbtree_rm_key(struct lbtree **tree,
                    unsigned int (*sel_key)(void *key, lbtree_index_t index),
                    void *key) {
  struct lbtree_leaf_pos leaf_pos = lbtree_leaf_pos(tree, sel_key, key, 0);
  struct lbtree_branch_pos branch_pos =
      lbtree_branch_pos(tree, sel_key, leaf_pos.leaf, key);
  lbtree_cut(branch_pos.ref, leaf_pos);
  return leaf_pos.leaf;
}

void lbtree_rm(struct lbtree **tree,
               unsigned int (*sel_node)(void *node, lbtree_index_t index),
               struct lbtree *node) {
  struct lbtree_branch_pos branch_pos =
      lbtree_branch_pos(tree, sel_node, node, node);
  struct lbtree_leaf_pos leaf_pos =
      lbtree_leaf_pos(branch_pos.ref, sel_node, node, branch_pos.parent);
  lbtree_cut(branch_pos.ref, leaf_pos);
}

static void *walk(struct lbtree *tree,
                  unsigned int (*sel_node)(void *node, lbtree_index_t index),
                  void *(*action)(void *node, void *closure), void *closure) {
  struct lbtree *children[LBTREE_LUT_SIZE];
  lbtree_index_t index = tree->index;

  unsigned int i;
  for (i = 0; i < LBTREE_LUT_SIZE; ++i) {
    struct lbtree *child = tree->children[i];
    children[i] = (sel_node(child, index) == i) ? child : 0;
  }

  for (i = 0; i < LBTREE_LUT_SIZE; ++i) {
    struct lbtree *child = children[i];
    if (child == 0) {
      continue;
    }
    void *r = (lbtree_index_gt(child->index, index) != 0)
                  ? walk(child, sel_node, action, closure)
                  : action(child, closure);
    if (r != 0) {
      return r;
    }
  }
  return 0;
}

void *lbtree_walk(struct lbtree *tree,
                  unsigned int (*sel_node)(void *node, lbtree_index_t index),
                  void *(*action)(void *node, void *closure), void *closure) {
  if (tree == 0) {
    return 0;
  }
  return walk(tree, sel_node, action, closure);
}

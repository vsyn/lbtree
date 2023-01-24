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

#ifndef LBTREE_H
#define LBTREE_H

#include <stdlib.h>

#define LBTREE_LUT_SIZE (2)

typedef unsigned long long int lbtree_index_t;

static inline int lbtree_index_gt(lbtree_index_t a, lbtree_index_t b) {
  return (a > b) ? 1 : 0;
}

static inline void lbtree_index_init(lbtree_index_t *index) { *index = 0; }

struct lbtree {
  struct lbtree *children[LBTREE_LUT_SIZE];
  lbtree_index_t index;
};

struct lbtree_leaf_pos {
  struct lbtree *cut;
  struct lbtree *leaf;
  struct lbtree **parent_ref;
  struct lbtree *grandparent;
};

struct lbtree_branch_pos {
  struct lbtree **ref;
  struct lbtree *parent;
};
/* Initialises the tree with one node - `lbtree_add` below should not be called
with a reference-to-zero `tree` parameter.
*/
void lbtree_init(struct lbtree *node);
/* Should be called when a matching key within the current tree is detected and
  needs to be replaced.
*/
void lbtree_repl(struct lbtree **tree,
                 unsigned int (*sel_node)(void *node, lbtree_index_t index),
                 struct lbtree *match, struct lbtree *node);
/* Adds an node to the tree. `node` must have index pre-populated, normal
  function of this add would include a lookup, compare to find the first index
  of divergence, and then add to insert into the tree.
*/
void lbtree_add(struct lbtree **tree,
                unsigned int (*sel_node)(void *node, lbtree_index_t index),
                struct lbtree *node);

/* Performs a lookup. Returns a pointer to the node associated with `key`.
 */
void *lbtree(struct lbtree *tree,
             unsigned int (*sel_key)(void *key, lbtree_index_t index),
             void *key);

/* Returns information about the position of the leaf associated with `key` in
 * the tree */
struct lbtree_leaf_pos
lbtree_leaf_pos(struct lbtree **tree,
                unsigned int (*sel)(void *key, lbtree_index_t index), void *key,
                struct lbtree *parent);

/* retrives information about the position of the branch associated with `key`
 * in the tree */
struct lbtree_branch_pos
lbtree_branch_pos(struct lbtree **tree,
                  unsigned int (*sel)(void *key, lbtree_index_t index),
                  struct lbtree *node, void *key);

void lbtree_cut(struct lbtree **branch_ref, struct lbtree_leaf_pos leaf_pos);

/* Removes a node by key. Returns a pointer to that node. Assumes the node
 * exists.
 */
void *lbtree_rm_key(struct lbtree **tree,
                    unsigned int (*sel_key)(void *key, lbtree_index_t index),
                    void *key);

/* Removes a node by pointer.
 */
void lbtree_rm(struct lbtree **tree,
               unsigned int (*sel_node)(void *node, lbtree_index_t index),
               struct lbtree *node);

/* Calls `action` once for each node in the tree, with a pointer to the node as
a first argument and `closure` as a second. The walk will stop when any action
returns a non-null pointer. Returns the action return value causing the walk
to stop, otherwise zero.
*/
void *lbtree_walk(struct lbtree *tree,
                  unsigned int (*sel_node)(void *node, lbtree_index_t index),
                  void *(*action)(void *node, void *closure), void *closure);

#endif

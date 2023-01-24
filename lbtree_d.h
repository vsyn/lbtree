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

#ifndef LBTREE_D_H
#define LBTREE_D_H

#include "lbtree.h"

#include <limits.h>

struct lbtree_d {
  struct lbtree base;
  unsigned char *key;
  void *val;
};

/*
Adds a key value pair to the tree, the value pointed to by `key` must persist
through any key value pair's lifetime as part of the tree.
Returns the value associated with a matching key in the tree (this key value
pair is replaced by this function), or zero if one is not found. `val` on memory
allocation error.
*/
void *lbtree_d_add(struct lbtree_d **size_tree, void *key,
                        lbtree_index_t key_size_bits, void *val);

static inline void *lbtree_dc_add(struct lbtree_d **size_tree, void *key,
                                 lbtree_index_t key_size, void *val) {
  lbtree_index_t key_size_bits = key_size * CHAR_BIT;
  return ((key_size_bits / CHAR_BIT) != key_size)
             ? key
             : lbtree_d_add(size_tree, key, key_size_bits, val);
}

/*
Performs a lookup on the tree.
Returns the value whose associated key matches the `key` argument, or zero if
one is not found.
*/
void *lbtree_d(struct lbtree_d *size_tree, void *key,
                    lbtree_index_t key_size_bits);

static inline void *lbtree_dc(struct lbtree_d *size_tree, void *key,
                             lbtree_index_t key_size) {
  lbtree_index_t key_size_bits = key_size * CHAR_BIT;
  return ((key_size_bits / CHAR_BIT) != key_size)
             ? 0
             : lbtree_d(size_tree, key, key_size_bits);
}

/*
Removes a single key value pair from the tree.
Returns the associated value if found and removed, otherwise zero.
*/
void *lbtree_d_rm(struct lbtree_d **size_tree, void *key,
                       lbtree_index_t key_size_bits);

static inline void *lbtree_dc_rm(struct lbtree_d **size_tree, void *key,
                                lbtree_index_t key_size) {
  lbtree_index_t key_size_bits = key_size * CHAR_BIT;
  return ((key_size_bits / CHAR_BIT) != key_size)
             ? 0
             : lbtree_d_rm(size_tree, key, key_size_bits);
}

/*
Calls `action` once for each key value pair in the tree. The walk will stop when
any action returns a non-null pointer. Returns the action return value causing
the walk to stop, otherwise zero.
*/
void *lbtree_d_walk(struct lbtree_d *size_tree,
                    void *(*action)(const void *key, void **val, void *closure),
                    void *closure);

/*
Frees all the nodes in the tree.
*/
void lbtree_d_free(struct lbtree_d *size_tree);

#endif

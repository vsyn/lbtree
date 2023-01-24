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

#include "lbtree_uint.h"

#include "limits.h"

static unsigned int sel_key(void *vkey, lbtree_index_t index) {
  return ((*(lbtree_uint_t *)vkey) >> index) & 1;
}

static unsigned int sel_node(void *vkey, lbtree_index_t index) {
  struct lbtree_uint *node = vkey;
  return sel_key(&node->key, index);
}

void *lbtree_uint_add(struct lbtree_uint **tree, struct lbtree_uint *node) {
  if (*tree == 0) {
    lbtree_init(&node->base);
    *tree = node;
    return 0;
  }

  struct lbtree_uint *match =
      (struct lbtree_uint *)lbtree(&(*tree)->base, &sel_key, &node->key);
  lbtree_index_t index = sizeof(unsigned long int) * CHAR_BIT;

  lbtree_uint_t da = node->key;
  lbtree_uint_t db = match->key;

  if (da == db) {
    lbtree_repl((struct lbtree **)tree, &sel_node, &match->base, &node->base);
    return match;
  }

  while (da != db) {
    da <<= 1;
    db <<= 1;
    --index;
  }
  node->base.index = index;
  lbtree_add((struct lbtree **)tree, &sel_node, &node->base);
  return 0;
}

void *lbtree_uint(struct lbtree_uint *tree, lbtree_uint_t key) {
  if (tree == 0) {
    return 0;
  }
  struct lbtree_uint *node =
      (struct lbtree_uint *)lbtree(&tree->base, &sel_key, &key);
  return (node->key == key) ? node : 0;
}

void lbtree_uint_rm(struct lbtree_uint **tree, struct lbtree_uint *node) {
  lbtree_rm((struct lbtree **)tree, &sel_node, &node->base);
}

void *lbtree_uint_rm_key(struct lbtree_uint **tree, lbtree_uint_t key) {
  return lbtree_rm_key((struct lbtree **)tree, &sel_key, &key);
}

void *lbtree_uint_walk(struct lbtree_uint *tree,
                       void *(*action)(void *node, void *closure),
                       void *closure) {
  return lbtree_walk(&tree->base, &sel_node, action, closure);
}

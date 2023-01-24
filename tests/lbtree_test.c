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

#include "../lbtree.h"

#include "lbtree_test.h"

#include <assert.h>
#include <limits.h>
#include <string.h>

struct lbtree_test {
  struct lbtree base;
  struct tree_test tt;
};

static struct tree_test *lbtree_test_node_tt(void *node) {
  struct lbtree_test *lbtt = node;
  return &lbtt->tt;
}

static void **lbtree_test_nodes_new(size_t size, size_t key_size) {
  size_t node_size = sizeof(struct lbtree_test) + key_size;
  void **nodes = malloc((node_size * size) + (sizeof(void *) * size));
  assert(nodes != 0);

  size_t i;
  for (i = 0; i < size; ++i) {
    nodes[i] = ((unsigned char *)(nodes + size)) + (node_size * i);
  }
  return nodes;
}

static void lbtree_test_nodes_del(void **nodes) { free(nodes); }

static unsigned int sel_key(void *key, lbtree_index_t index) {
  lbtree_index_t char_index = index / CHAR_BIT;
  return (char_index < (*(size_t *)key + sizeof(size_t)))
             ? (((unsigned char *)key)[char_index] >> (index % CHAR_BIT)) & 1
             : 0;
}

static unsigned int sel_node(void *v_node, lbtree_index_t index) {
  struct lbtree_test *node = v_node;
  return sel_key(&node->tt.key, index);
}

static void *lbtree_test_init(void) { return 0; }

static void *lbtree_test_add(void **v_tree, void *v_node) {
  struct lbtree **tree = (struct lbtree **)v_tree;
  struct lbtree_test *node = v_node;
  if (*tree == 0) {
    lbtree_init(&node->base);
    *tree = &node->base;
    return 0;
  }

  struct lbtree_test *match =
      (struct lbtree_test *)lbtree(*tree, &sel_key, &node->tt.key);
  unsigned char *ka = (unsigned char *)&node->tt.key;
  unsigned char *kb = (unsigned char *)&match->tt.key;
  size_t size = node->tt.key.size + sizeof(node->tt.key.size);
  lbtree_index_t index = 0;
  while ((index < size) && (*ka == *kb)) {
    ++ka;
    ++kb;
    ++index;
  }
  if (index == size) {
    lbtree_repl(tree, &sel_node, &match->base, &node->base);
    return match;
  }
  unsigned char da = *ka;
  unsigned char db = *kb;
  index *= CHAR_BIT;
  index += CHAR_BIT;
  while (da != db) {
    da <<= 1;
    db <<= 1;
    --index;
  }
  node->base.index = index;
  lbtree_add(tree, &sel_node, &node->base);
  return 0;
}

static void *lbtree_test_lookup(void *tree, struct tree_test *tt) {
  if (tree == 0) {
    return 0;
  }
  struct tree_test_key *key = &tt->key;
  struct lbtree_test *node = (struct lbtree_test *)lbtree(tree, &sel_key, key);
  return ((node->tt.key.size == key->size) &&
          (memcmp(node->tt.key.buf, key->buf, key->size) == 0))
             ? node
             : 0;
}

static void lbtree_test_rm(void **tree, void *node) {
  lbtree_rm((struct lbtree **)tree, &sel_node, (struct lbtree *)node);
}

struct walk_closure {
  void *(*action)(const void *key, void **val, void *closure);
  void *closure;
};

static void *walk_action(void *node, void *closure) {
  struct walk_closure *walk_closure = closure;
  return walk_closure->action(node, &node, walk_closure->closure);
}

static void *lbtree_test_walk(void *tree,
                              void *(*action)(const void *key, void **val,
                                              void *closure),
                              void *closure) {
  struct walk_closure walk_closure = {.action = action, .closure = closure};
  return lbtree_walk(tree, &sel_node, &walk_action, &walk_closure);
}

static void lbtree_test_del(void *vlbt) { (void)vlbt; }

const struct tree_test_iface lbtree_test_iface = {
    .node_tt = &lbtree_test_node_tt,
    .nodes_new = &lbtree_test_nodes_new,
    .nodes_del = &lbtree_test_nodes_del,
    .init = &lbtree_test_init,
    .add = &lbtree_test_add,
    .lookup = &lbtree_test_lookup,
    .rm = &lbtree_test_rm,
    .walk = &lbtree_test_walk,
    .del = &lbtree_test_del};

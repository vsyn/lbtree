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

#include "lbtree_d_test.h"

#include "../lbtree_d.h"

#include <assert.h>
#include <limits.h>

static struct tree_test *lbtree_d_test_node_tt(void *node) { return node; }

static void **lbtree_d_test_nodes_new(size_t size, size_t key_size) {
  size_t node_size = sizeof(struct tree_test) + key_size;
  void **nodes = malloc((node_size * size) + (sizeof(void *) * size));
  assert(nodes != 0);

  size_t i;
  for (i = 0; i < size; ++i) {
    nodes[i] = ((unsigned char *)(nodes + size)) + (node_size * i);
  }
  return nodes;
}

static void lbtree_d_test_nodes_del(void **nodes) { free(nodes); }

static void *lbtree_d_test_init(void) { return 0; }

static void *lbtree_d_test_add(void **tree, void *v_tt) {
  struct tree_test *tt = v_tt;
  void *r =
      lbtree_dc_add((struct lbtree_d **)tree, tt->key.buf, tt->key.size, v_tt);
  return r;
}

static void *lbtree_d_test_lookup(void *tree, struct tree_test *node) {
  return lbtree_dc(tree, node->key.buf, node->key.size);
}

static void lbtree_d_test_rm(void **tree, void *v_node) {
  struct tree_test *node = v_node;
  lbtree_dc_rm((struct lbtree_d **)tree, node->key.buf, node->key.size);
}

struct walk_closure {
  void *(*action)(const void *key, void **val, void *closure);
  void *closure;
};

static void *walk_action(const void *key, void **val, void *v_closure) {
  struct walk_closure *closure = v_closure;
  return closure->action(key, val, closure->closure);
}

static void *lbtree_d_test_walk(void *tree,
                                void *(*action)(const void *key, void **val,
                                                void *closure),
                                void *closure) {
  struct walk_closure walk_closure = {.action = action, .closure = closure};
  return lbtree_d_walk(tree, &walk_action, &walk_closure);
}

static void lbtree_d_test_del(void *test) { lbtree_d_free(test); }

const struct tree_test_iface lbtree_d_test_iface = {
    .node_tt = &lbtree_d_test_node_tt,
    .nodes_new = &lbtree_d_test_nodes_new,
    .nodes_del = &lbtree_d_test_nodes_del,
    .init = &lbtree_d_test_init,
    .add = &lbtree_d_test_add,
    .lookup = &lbtree_d_test_lookup,
    .rm = &lbtree_d_test_rm,
    .walk = &lbtree_d_test_walk,
    .del = &lbtree_d_test_del};

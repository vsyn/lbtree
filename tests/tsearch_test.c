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

#include "tsearch_test.h"

#include <assert.h>
#include <search.h>
#include <string.h>

static struct tree_test *tsearch_test_node_tt(void *node) { return node; }

static void **tsearch_test_nodes_new(size_t size, size_t key_size) {
  size_t node_size = sizeof(struct tree_test) + key_size;
  void **nodes = malloc((node_size * size) + (sizeof(void *) * size));
  assert(nodes != 0);

  size_t i;
  for (i = 0; i < size; ++i) {
    nodes[i] = (unsigned char *)(nodes + size) + (node_size * i);
  }
  return nodes;
}

static void tsearch_test_nodes_del(void **nodes) { free(nodes); }

static int cmp(const void *v_l, const void *v_r) {
  const struct tree_test *l = v_l;
  const struct tree_test *r = v_r;
  if (l->key.size != r->key.size) {
    return l->key.size - r->key.size;
  }
  return memcmp(l->key.buf, r->key.buf, l->key.size);
}

static void *tsearch_test_init(void) { return 0; }

static void *tsearch_test_add(void **tree, void *tt) {
  struct tree_test **r = tfind(tt, tree, &cmp);
  struct tree_test *ret = 0;
  if (r != 0) {
    ret = *r;
    tdelete(ret, tree, &cmp);
  }
  tsearch(tt, tree, &cmp);
  return ret;
}

static void *tsearch_test_lookup(void *tree, struct tree_test *key) {
  struct tree_test **r = tfind(key, &tree, &cmp);
  return (r == 0) ? 0 : *r;
}

static void tsearch_test_rm(void **test, void *tt) { tdelete(tt, test, &cmp); }

static void tsearch_test_del(void *test) { (void)test; }

const struct tree_test_iface tsearch_test_iface = {
    .node_tt = &tsearch_test_node_tt,
    .nodes_new = &tsearch_test_nodes_new,
    .nodes_del = &tsearch_test_nodes_del,
    .init = &tsearch_test_init,
    .add = &tsearch_test_add,
    .lookup = &tsearch_test_lookup,
    .rm = &tsearch_test_rm,
    .del = &tsearch_test_del};

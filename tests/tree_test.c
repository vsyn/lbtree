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

#include "tree_test.h"

#include "everand/everand.h"

#include <assert.h>
#include <string.h>

static void tree_test_rand_node(struct tree_test *tt, size_t min_key_size,
                                size_t max_key_size) {
  tt->key.size = everand(max_key_size - min_key_size) + min_key_size;
  everand_arr(tt->key.buf, tt->key.size);
  tt->active = 0;
}

static int ttk_cmp(struct tree_test_key *a, struct tree_test_key *b) {
  size_t a_size = a->size;
  size_t b_size = b->size;
  if (a_size != b_size) {
    return (a_size < b_size) ? -1 : 1;
  }
  size_t i = 0;
  while (i < a_size) {
    unsigned char *a_buf = a->buf;
    unsigned char *b_buf = b->buf;
    if (a_buf[i] != b_buf[i]) {
      return a_buf[i] - b_buf[i];
    }
    ++i;
  }
  return 0;
}

struct tree_test_state {
  void **nodes;
  void *tree;
  size_t prob;
  size_t size;
  size_t max_size;
};

/* start off with a certanty of adding, then reduce the probability of adding
  and increase the probabliity of removing */
static int tree_test_rep(const struct tree_test_iface *iface,
                         struct tree_test_state *state) {
  if (everand(state->max_size - 1) >= state->prob) {
    /* add to the tree, use nodes[prob] */
    void *node = state->nodes[state->prob];
    struct tree_test *tt = iface->node_tt(node);
    tt->id = state->prob;
    void *vrepl = iface->add(&state->tree, node);
    if (vrepl != 0) {
      struct tree_test *repl = iface->node_tt(vrepl);
      assert(repl->key.size == tt->key.size);
      assert(memcmp(repl->key.buf, tt->key.buf, tt->key.size) == 0);
      repl->active = 0;
    } else {
      ++state->size;
    }
    tt->active = 1;
    ++state->prob;
  } else if (state->size != 0) {
    /* lookup */
    size_t index = 0;
    while (index < state->prob) {
      void *node = state->nodes[index];
      struct tree_test *tt = iface->node_tt(node);
      size_t tt_size = sizeof(*tt) + tt->key.size;
      struct tree_test *ttp = malloc(tt_size);
      assert(ttp != 0);
      memcpy(ttp, tt, tt_size);
      void *found = iface->lookup(state->tree, ttp);
      free(ttp);
      if (found == 0) {
        assert(tt->active == 0);
      } else if (found != node) {
        struct tree_test *ftt = iface->node_tt(found);
        assert(ftt->key.size == tt->key.size);
        assert(memcmp(ftt->key.buf, tt->key.buf, ftt->key.size) == 0);
      }
      ++index;
    }
    /* remove */
    index = everand(state->size - 1);
    void *node = state->nodes[index];
    struct tree_test *old = iface->node_tt(node);
    while (old->active == 0) {
      /* has previously been removed */
      ++index;
      index %= state->prob;
      node = state->nodes[index];
      old = iface->node_tt(node);
    }
    iface->rm(&state->tree, node);
    old->active = 0;
    --state->size;
  } else {
    return 0;
  }
  return 1;
}

void tree_test(const struct tree_test_iface *iface,
               struct tree_test_config *config) {
  void **nodes = iface->nodes_new(config->max_size, config->max_key_size);

  size_t i;
  for (i = 0; i < config->max_size; ++i) {
    tree_test_rand_node(iface->node_tt(nodes[i]), config->min_key_size,
                        config->max_key_size);
  }

  if (config->sort != 0) {
    /* insertion sort */
    unsigned int a_offs = config->sort & 1;
    unsigned int b_offs = a_offs ^ 1;
    for (i = 1; i < config->max_size; ++i) {
      size_t j = i;
      while (j > 0) {
        struct tree_test *a = iface->node_tt(nodes[j - a_offs]);
        struct tree_test *b = iface->node_tt(nodes[j - b_offs]);
        if (ttk_cmp(&a->key, &b->key) >= 0) {
          break;
        }
        void *tmp = nodes[j - 1];
        nodes[j - 1] = nodes[j];
        nodes[j] = tmp;
        --j;
      }
    }
  }

  struct tree_test_state state = {.nodes = nodes,
                                  .tree = iface->init(),
                                  .prob = 0,
                                  .size = 0,
                                  .max_size = config->max_size};

  while (tree_test_rep(iface, &state) != 0) {
  }

  iface->del(state.tree);
  iface->nodes_del(nodes);
}

static void *tree_test_rand_tree(const struct tree_test_iface *iface,
                                 void **nodes, size_t *size,
                                 size_t min_key_size, size_t max_key_size) {
  void *tree = iface->init();
  *size = everand(*size - 1);
  size_t i;
  for (i = 0; i < *size; ++i) {
    struct tree_test *tt = iface->node_tt(nodes[i]);
    tree_test_rand_node(tt, min_key_size, max_key_size);
    tt->id = i;
    tt->active = 1;
    void *vrepl = iface->add(&tree, nodes[i]);
    if (vrepl != 0) {
      struct tree_test *repl = iface->node_tt(vrepl);
      repl->active = 0;
    }
  }
  return tree;
}

static void *action(const void *key, void **val, void *closure) {
  (void)key;
  struct tree_test_iface *iface = closure;
  struct tree_test *tt = iface->node_tt(*val);
  --tt->active;
  return 0;
}

void tree_test_walk(const struct tree_test_iface *iface,
                    struct tree_test_config *config) {
  size_t size = config->max_size;
  void **nodes = iface->nodes_new(config->max_size, config->max_key_size);
  void *tree = tree_test_rand_tree(iface, nodes, &size, config->min_key_size,
                                   config->max_key_size);
  iface->walk(tree, &action, (void *)iface);
  size_t i;
  for (i = 0; i < size; ++i) {
    assert(iface->node_tt(nodes[i])->active == 0);
  }
  iface->del(tree);
  iface->nodes_del(nodes);
}

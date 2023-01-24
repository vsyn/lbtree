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

#ifndef TREE_TEST_H
#define TREE_TEST_H

#include <stdlib.h>

struct tree_test_key {
  size_t size;
  unsigned char buf[];
};

struct tree_test {
  size_t id;
  unsigned char active;
  struct tree_test_key key;
};

struct tree_test_config {
  size_t max_size;
  size_t min_key_size;
  size_t max_key_size;
  unsigned char sort;
};

struct tree_test_iface {

  struct tree_test *(*node_tt)(void *node);
  void **(*nodes_new)(size_t size, size_t key_size);
  void (*nodes_del)(void **nodes);
  /* returns empty tree object */
  void *(*init)(void);
  /* adds key value pair to tree, returns any overwritten value */
  void *(*add)(void **tree, void *node);
  void *(*lookup)(void *tree, struct tree_test *key);
  /* remove key value pair from tree */
  void (*rm)(void **tree, void *node);
  void *(*walk)(void *tree,
                void *(*action)(const void *key, void **val, void *closure),
                void *closure);
  void (*del)(void *tree);
};

void tree_test(const struct tree_test_iface *iface,
               struct tree_test_config *config);
void tree_test_walk(const struct tree_test_iface *iface,
                    struct tree_test_config *config);
#endif

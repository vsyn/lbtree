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

#include "./everand/everand.h"
#include "lbtree_d_test.h"
#include "lbtree_test.h"
#include "tsearch_test.h"

#include <stdio.h>

#define SEED (16)
#define TEST_COUNT (32)

void test_multiple(const struct tree_test_iface *iface,
                   struct tree_test_config *config, unsigned int test_count) {
  everand_seed(SEED);

  unsigned int i;
  for (i = 0; i < test_count; ++i) {
    tree_test(iface, config);
  }
}

void test_walk_multiple(const struct tree_test_iface *iface,
                        struct tree_test_config *config,
                        unsigned int test_count) {
  everand_seed(SEED);

  unsigned int i;
  for (i = 0; i < test_count; ++i) {
    tree_test_walk(iface, config);
  }
}

int main(int argc, char *argv[]) {
  (void)argc;
  (void)argv;
  struct tree_test_config config = {
      .max_size = 4096, .min_key_size = 0, .max_key_size = 8, .sort = 0};

  test_multiple(&lbtree_test_iface, &config, TEST_COUNT);
  test_multiple(&lbtree_d_test_iface, &config, TEST_COUNT);
  test_multiple(&tsearch_test_iface, &config, TEST_COUNT);

  test_walk_multiple(&lbtree_test_iface, &config, TEST_COUNT);
  test_walk_multiple(&lbtree_d_test_iface, &config, TEST_COUNT);

  return 0;
}

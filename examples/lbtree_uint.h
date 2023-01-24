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

#ifndef LBTREE_UINT_H
#define LBTREE_UINT_H

#include "lbtree.h"

typedef unsigned long int lbtree_uint_t;

struct lbtree_uint {
  struct lbtree base;
  lbtree_uint_t key;
};

void *lbtree_uint_add(struct lbtree_uint **tree, struct lbtree_uint *node);
void *lbtree_uint(struct lbtree_uint *tree, lbtree_uint_t key);
void lbtree_uint_rm(struct lbtree_uint **tree, struct lbtree_uint *node);
void *lbtree_uint_rm_key(struct lbtree_uint **tree, lbtree_uint_t key);
void *lbtree_uint_walk(struct lbtree_uint *tree,
                       void *(*action)(void *node, void *closure),
                       void *closure);

#endif

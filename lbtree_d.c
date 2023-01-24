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

/* Wrapper around lbtree to give an easier to use, dynamically allocated, lookup
   tree */

#include "lbtree_d.h"

struct lbtree_d_size {
  struct lbtree_d base;
  lbtree_index_t key_size_bits;
};

static unsigned int sel_key(void *key, lbtree_index_t index) {
  return (((unsigned char *)key)[index / CHAR_BIT] >> (index % CHAR_BIT)) & 1;
}

static unsigned int sel_node(void *v_node, lbtree_index_t index) {
  struct lbtree_d *node = v_node;
  return sel_key(node->key, index);
}

static inline unsigned int get_shift(unsigned char a, unsigned char b) {
  unsigned int shift = CHAR_BIT;
  while (a != b) {
    a <<= 1;
    b <<= 1;
    --shift;
  }
  return shift;
}

void *lbtree_d_add(struct lbtree_d **size_tree, void *key,
                        lbtree_index_t key_size_bits, void *val) {
  if (*size_tree == 0) {
    struct lbtree_d_size *size_node = malloc(sizeof(*size_node));
    if (size_node == 0) {
      return val;
    }
    size_node->key_size_bits = key_size_bits;
    size_node->base.key = (unsigned char *)&size_node->key_size_bits;
    size_node->base.val = 0;
    lbtree_init(&size_node->base.base);
    *size_tree = &size_node->base;

    struct lbtree_d *key_node = malloc(sizeof(*key_node));
    if (key_node == 0) {
      free(size_node);
      return val;
    }
    key_node->key = key;
    key_node->val = val;
    lbtree_init(&key_node->base);
    size_node->base.val = &key_node->base;
    return 0;
  }

  struct lbtree_d_size *size_best = (struct lbtree_d_size *)lbtree(
      (struct lbtree *)*size_tree, &sel_key, &key_size_bits);

  unsigned char *ka;
  unsigned char *kb;
  if (size_best->key_size_bits == key_size_bits) {
    /* key_size already exists in size tree, add to key tree only */
    if (key_size_bits == 0) {
      struct lbtree_d *key_best = size_best->base.val;
      void *old_val = key_best->val;
      key_best->key = key;
      key_best->val = val;
      return old_val;
    }

    struct lbtree_d *key_best = lbtree(size_best->base.val, &sel_key, key);

    ka = key;
    kb = key_best->key;
    lbtree_index_t index = 0;
    lbtree_index_t whole_char_size = key_size_bits / CHAR_BIT;
    while ((index < whole_char_size) && (*ka == *kb)) {
      ++ka;
      ++kb;
      ++index;
    }

    index *= CHAR_BIT;

    if (index != key_size_bits) {
      index += get_shift(*ka, *kb);
    }

    if (index == key_size_bits) {
      void *old_val = key_best->val;
      key_best->key = key;
      key_best->val = val;
      return old_val;
    }

    struct lbtree_d *key_node = malloc(sizeof(*key_node));
    if (key_node == 0) {
      return val;
    }
    key_node->key = key;
    key_node->val = val;

    key_node->base.index = index;
    lbtree_add((struct lbtree **)&size_best->base.val, &sel_node,
               &key_node->base);
  } else {
    /* key_size_bits not yet in size tree, add to size tree and to key tree */
    struct lbtree_d_size *size_node = malloc(sizeof(*size_node));
    if (size_node == 0) {
      return val;
    }
    size_node->key_size_bits = key_size_bits;
    size_node->base.key = (unsigned char *)&size_node->key_size_bits;
    size_node->base.val = 0;

    ka = size_node->base.key;
    kb = size_best->base.key;
    lbtree_index_t index = 0;
    while (*ka == *kb) {
      ++ka;
      ++kb;
      index += CHAR_BIT;
    }

    index += get_shift(*ka, *kb);

    struct lbtree_d *key_node = malloc(sizeof(*key_node));
    if (key_node == 0) {
      return val;
    }
    key_node->key = key;
    key_node->val = val;

    size_node->base.base.index = index;
    lbtree_add((struct lbtree **)size_tree, &sel_node, &size_node->base.base);

    lbtree_init(&key_node->base);
    size_node->base.val = &key_node->base;
  }
  return 0;
}

static int match(unsigned char *ka, unsigned char *kb,
                 lbtree_index_t size_bits) {
  while (size_bits > CHAR_BIT) {
    if (*ka != *kb) {
      return -1;
    }
    ++ka;
    ++kb;
    size_bits -= CHAR_BIT;
  }
  unsigned char mask = (1 << size_bits) - 1;
  return ((*ka & mask) == (*kb & mask)) ? 0 : -1;
}

void *lbtree_d(struct lbtree_d *size_tree, void *key,
                    lbtree_index_t key_size_bits) {
  struct lbtree_d *size_best =
      lbtree(&size_tree->base, &sel_key, &key_size_bits);
  if ((*(lbtree_index_t *)size_best->key) != key_size_bits) {
    return 0;
  }
  if (key_size_bits == 0) {
    struct lbtree_d *key_best = size_best->val;
    return key_best->val;
  }
  struct lbtree_d *key_best = lbtree(size_best->val, &sel_key, key);
  return (match(key_best->key, key, key_size_bits) == 0) ? key_best->val : 0;
}

static void rm_leaf_pos(struct lbtree **tree,
                        unsigned int (*sel)(void *tree, lbtree_index_t index),
                        void *key, struct lbtree_leaf_pos leaf_pos) {
  struct lbtree_branch_pos branch_pos =
      lbtree_branch_pos(tree, sel, leaf_pos.leaf, key);
  lbtree_cut(branch_pos.ref, leaf_pos);
}

void *lbtree_d_rm(struct lbtree_d **size_tree, void *key,
                       lbtree_index_t key_size_bits) {
  if (*size_tree == 0) {
    return 0;
  }
  struct lbtree_leaf_pos size_best_family =
      lbtree_leaf_pos((struct lbtree **)size_tree, &sel_key, &key_size_bits, 0);
  struct lbtree_d_size *size_best =
      (struct lbtree_d_size *)size_best_family.leaf;

  if (size_best->key_size_bits != key_size_bits) {
    return 0;
  }

  if (key_size_bits == 0) {
    struct lbtree_d *key_best = size_best->base.val;
    void *old_val = key_best->val;
    free(key_best);
    rm_leaf_pos((struct lbtree **)size_tree, &sel_key, &key_size_bits,
                size_best_family);
    free(size_best);
    return old_val;
  }

  struct lbtree **key_tree = (struct lbtree **)&size_best->base.val;
  struct lbtree_leaf_pos key_best_family =
      lbtree_leaf_pos(key_tree, &sel_key, key, 0);
  struct lbtree_d *leaf = (struct lbtree_d *)key_best_family.leaf;
  if (match(leaf->key, key, key_size_bits) != 0) {
    return 0;
  }
  rm_leaf_pos(key_tree, &sel_key, key, key_best_family);

  void *val = leaf->val;
  free(leaf);

  if (*key_tree == 0) {
    /* key tree empty, also remove from size_tree */
    rm_leaf_pos((struct lbtree **)size_tree, &sel_key, &key_size_bits,
                size_best_family);
    free(size_best);
  }
  return val;
}

struct walk_closure {
  void *(*action)(const void *key, void **val, void *closure);
  void *closure;
};

static void *walk_key_action(void *node, void *v_closure) {
  struct lbtree_d *dyn_node = node;
  struct walk_closure *closure = v_closure;
  return closure->action(dyn_node->key, &dyn_node->val, closure->closure);
}

static void *walk_size_action(void *node, void *v_closure) {
  struct lbtree_d_size *dyn_size = node;
  struct lbtree_d *key_tree = dyn_size->base.val;
  if (dyn_size->key_size_bits == 0) {
    struct walk_closure *closure = v_closure;
    return closure->action(key_tree->key, &key_tree->val, closure->closure);
  }
  return lbtree_walk(&key_tree->base, &sel_node, &walk_key_action, v_closure);
}

void *lbtree_d_walk(struct lbtree_d *size_tree,
                    void *(*action)(const void *key, void **val, void *closure),
                    void *v_closure) {
  struct walk_closure closure = {.action = action, .closure = v_closure};
  return lbtree_walk(&size_tree->base, &sel_node, &walk_size_action, &closure);
}

static void *free_key_action(void *node, void *closure) {
  (void)closure;
  free(node);
  return 0;
}

static void *free_size_action(void *node, void *closure) {
  (void)closure;
  struct lbtree_d_size *dyn_size = node;
  struct lbtree_d *key_tree = dyn_size->base.val;
  if (dyn_size->key_size_bits == 0) {
    free(key_tree);
  } else {
    lbtree_walk(&key_tree->base, &sel_node, &free_key_action, 0);
  }
  free(node);
  return 0;
}

void lbtree_d_free(struct lbtree_d *size_tree) {
  lbtree_walk(&size_tree->base, &sel_node, &free_size_action, 0);
}

# lbtree

lbtree is a lookup tree library. It can be configured to use linear memory and handle keys of varying and unlimited sizes.

lbtree offers 2 interfaces, `lbtree_d` a higher level interface that does dynamic allocation of memory, and `lbtree` that leaves it to the user to allocate nodes and provide methods for key comparison. `lbtree_d` is a wrapper around `lbtree`.

## lbtree_d

lbtree_d provides an interface that represents the key value pair using a pointer and size for the key and a pointer for the value.

``` C
#include "lbtree_d.h"

#include <assert.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    /* define 2 different keys of (potentially) different sizes */
    int key1 = 1;
    char val1[] = "value 1";

    char key2[] = "key 2";
    char val2[] = "value 2";

    /* initialise empty tree */
    struct lbtree_d *tree = 0;

    /* add the 1st key value pair to the tree */
    assert(lbtree_dc_add(&tree, &key1, sizeof(key1), val1) != &key1);

    /* add the 2nd key value pair to the tree */
    assert(lbtree_dc_add(&tree, key2, sizeof(key2), val2) != key2);

    int lookup_key1 = 1;
    char lookup_key2[] = "key 2";

    /* lookup the value associated with key 1 */
    char *lookup_val = lbtree_dc(tree, &lookup_key1, sizeof(lookup_key1));

    /* prints "value 1" */
    printf("%s\n", lookup_val);

    /* lookup the value associated with key 2 */
    lookup_val = lbtree_dc(tree, &lookup_key2, sizeof(lookup_key2));

    /* prints "value 2" */
    printf("%s\n", lookup_val);

    /* remove the 2nd key value pair from the tree */
    char *rm_val = lbtree_dc_rm(&tree, lookup_key2, sizeof(lookup_key2));

    /* prints "value 2" */
    printf("%s\n", rm_val);

    /* check the 2nd key value pair is removed */
    assert(lbtree_dc(tree, &lookup_key2, sizeof(lookup_key2)) == 0);

    lbtree_d_free(tree);
}
```

## lbtree

lbtree is designed for use with a struct contatining the key and value (or their references), and whose first element is a `struct lbtree`. See `struct lbtree_d` in `lbtree_d.h` for an example.

Where `sel` function pointers are passed as arguments, these should point to functions that return the value of a single bit in the key at index `index` of a key, generaly `sel_key` expects to receive a pointer to the key as its first parameter, and `sel_node` expects to receive a pointer to a user specified tree node structure conforming to the requirements in the paragraph directly above. This may depend on what is provided by the user (for example if you had a tree node structure and you wanted to see if there was a matching key in the tree, you could call `lbtree` with the `sel_node` function and the node as the `key` argument).

The default bit index is specified as an `unsigned long long int` this supports keys of size less than `ULLONG_MAX / CHAR_BIT` characters, this is likely a large enough number but theoretically may not be able to represent all that `size_t` can. If this matters to you then an alternate index type can be specified by modifying the `lbtree_index_t` typedef, and possibly the `lbtree_index_gt` and `lbtree_index_init` functions in `lbtree.h`.

See `lbtree_uint.c` in the `examples` directory for an example wrapper for an integer key.

## Compilation

There is a makefile that compiles the tests and a static library for lbtree, however if you wish to add this to your project, all that is required is to compile `lbtree.c` (and `lbtree_d.c` if you are using that functionality) and provide your compiler access to the header(s) in the `lbtree` direcotry.
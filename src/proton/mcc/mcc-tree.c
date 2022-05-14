#include <stdlib.h>
#include <string.h>
#include "mcc-tree.h"


static char *mccstrdup(const char *s)
{
    size_t N = strlen(s) + 1;
    char *c, *out = malloc(sizeof *out * N);
    if (!out) {
        return NULL;
    }
    c = out;
    do {
        *c = *s;
        c++;
        s++;
        N--;
    } while (N);
    return out;
}

static int strsegcmp(const char *cmpstr, const char *nptr, const char *const endptr)
{
    while (nptr < endptr) {
        if (*cmpstr != *nptr) {
            return -1;
        } else {
            cmpstr++;
            nptr++;
        }
    }
    return *cmpstr != '\0';
}

struct mcc_map *new_mcc_map()
{
    struct mcc_map *map = malloc(sizeof *map);
    if (!map) {
        return NULL;
    }
    map->root = NULL;
    return map;
}

static void free_mcc_map_node(struct mcc_map_node *node)
{
    if (!node) {
        return;
    }
    free_mcc_map_node(node->left);
    free_mcc_map_node(node->right);
    free(node->key);
    free(node);
}

void free_mcc_map(struct mcc_map *map)
{
    free_mcc_map_node(map->root);
    free(map);
}

static struct mcc_map_node *new_mcc_map_node(const char *key, const char *keyend)
{
    size_t eq_sgn = keyend - key;
    struct mcc_map_node *node = malloc(sizeof *node);
    if (!node) {
        return NULL;
    }
    node->key = mccstrdup(key);
    if (!node->key) {
        free(node);
        return NULL;
    }
    node->key[eq_sgn] = '\0';
    node->value = node->key + eq_sgn + 1;
    node->left = NULL;
    node->right = NULL;
    return node;
}

static int mcc_insert_recur(struct mcc_map_node *node, const char *key, const char *keyend)
{
    struct mcc_map_node **construct;
    switch (strsegcmp(node->key, key, keyend)) {
    case -1:
        if (node->left) {
            return mcc_insert_recur(node->left, key, keyend);
        } else {
            construct = &node->left;
            break;
        }
    case 0:
        return 0;
    case 1:
        if (node->right) {
            return mcc_insert_recur(node->right, key, keyend);
        } else {
            construct = &node->right;
            break;
        }
    }
    *construct = new_mcc_map_node(key, keyend);
    return *construct == NULL;
}

int insert_mcc_kvpair(struct mcc_map *map, const char *key, const char *keyend)
{
    if (!map->root) {
        map->root = new_mcc_map_node(key, keyend);
        return map->root == NULL;
    } else {
        return mcc_insert_recur(map->root, key, keyend);
    }
}

const char *mcc_map_lookup(struct mcc_map *map, const char *key)
{
    struct mcc_map_node *node = map->root;
    while (node) {
        switch (strcmp(node->key, key)) {
        case -1:
            node = node->left;
            break;
        case 0:
            return node->value;
        case 1:
            node = node->right;
            break;
        }
    }
    return NULL;
}

void clear_mcc_map(struct mcc_map *map)
{
    free_mcc_map_node(map->root);
    map->root = NULL;
}

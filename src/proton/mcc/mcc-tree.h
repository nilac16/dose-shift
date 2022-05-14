#pragma once

#ifndef MCC_TREE_MAP_H
#define MCC_TREE_MAP_H


struct mcc_map {
    struct mcc_map_node {
        char *key, *value;
        struct mcc_map_node *left, *right;
    } *root;
};

struct mcc_map *new_mcc_map();

void free_mcc_map(struct mcc_map *map);

/** It is assumed that the value string begins immediately after the keyend 
 *  char
 */
int insert_mcc_kvpair(struct mcc_map *map, const char *key, const char *value);

const char *mcc_map_lookup(struct mcc_map *map, const char *key);

void clear_mcc_map(struct mcc_map *map);


void PRINT_ALL(const struct mcc_map *map);

#endif /* MCC_TREE_MAP_H */

//
// Created by Никита Кобик on 15.11.2024.
//

#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdbool.h>

typedef struct cache_block cache_block;
typedef struct file_cache file_cache;


struct cache_block {
    off_t offset;
    void *data;
    size_t block_size;
    int usage_frequency;
    bool is_dirty;
    cache_block *next_block;
};

struct file_cache {
    int fd;
    size_t block_size;
    size_t max_blocks;
    off_t current_offset;
    size_t current_blocks;
    cache_block *head;
};

int lfu_cache_evict(file_cache *cache);
int create_cache_block(file_cache *cache, const void *buf, ssize_t count, off_t offset);

#endif //UTILS_H

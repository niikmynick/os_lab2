//
// Created by Никита Кобик on 15.11.2024.
//

#include "utils.h"


int lfu_cache_evict(file_cache *cache) {
    if (!cache) {
        perror("[!] Get cache");
        return -1;
    }

    cache_block *prev = NULL;
    cache_block *least_used = cache->head;
    cache_block *current = cache->head;

    while (current->next_block) {
        if (current->next_block->usage_frequency < least_used->usage_frequency) {
            prev = current;
            least_used = current->next_block;
        }

        current = current->next_block;
    }

    if (least_used->is_dirty) {
        const ssize_t bytes_written = pwrite(cache->fd, least_used->data, least_used->block_size, least_used->offset);
        if (bytes_written < 0) {
            perror("[!] Write block to disk while eviction");
        }
    }

    if (prev) {
        prev->next_block = least_used->next_block;
    } else {
        cache->head = least_used->next_block;
    }

    free(least_used->data);
    free(least_used);

    cache->current_blocks--;

    return 0;
}


int create_cache_block(file_cache *cache, const void *buf, ssize_t count, off_t offset) {
    void *block_data = malloc(cache->block_size);
    if (!block_data) {
        perror("[!] Allocate memory for cache block");
        return -1;
    }

    memcpy(block_data, buf, count);

    cache_block *new_block = malloc(sizeof(cache_block));
    if (!new_block) {
        perror("[!] Allocate memory for cache block");
        free(block_data);
        return -1;
    }

    new_block->offset = offset;
    new_block->data = block_data;
    new_block->block_size = count;
    new_block->usage_frequency = 1;
    new_block->is_dirty = false;
    new_block->next_block = cache->head;

    cache->head = new_block;
    cache->current_blocks++;

    return 0;
}
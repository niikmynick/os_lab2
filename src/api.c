//
// Created by Никита Кобик on 15.11.2024.
//

#include "api.h"



file_cache* lab2_open(const char *path, const size_t block_size, const size_t max_blocks) {
    const int fd = open(path, O_RDWR);

    if (fd < 0) {
        perror("[!] Open file");
        return NULL;
    }

    if (fcntl(fd, F_NOCACHE, 1) < 0) {
        perror("[!] Disable system caching");
        close(fd);
        return NULL;
    }

    file_cache *cache = malloc(sizeof(file_cache));

    if (!cache) {
        perror("[!] Allocate memory for cache");
        close(fd);
        return NULL;
    }

    cache->fd = fd;
    cache->block_size = block_size;
    cache->max_blocks = max_blocks;
    cache->current_blocks = 0;
    cache->head = NULL;

    return cache;
}


int lab2_close(file_cache *cache) {
    if (!cache) {
        perror("[!] Get cache");
        return -1;
    }

    cache_block *current = cache->head;

    while (current) {
        cache_block *next = current->next_block;

        if (current->is_dirty) {
            ssize_t bytes_written = pwrite(cache->fd, current->data, current->block_size, current->offset);

            if (bytes_written < 0) {
                perror("[!] Write block to disk while closing");
            }
        }

        free(current->data);
        free(current);
        current = next;
    }

    close(cache->fd);
    free(cache);

    return 0;
}


int lab2_fsync(file_cache *cache) {
    if (!cache) {
        perror("[!] Get cache");
        return -1;
    }

    cache_block *current = cache->head;

    while (current) {
        if (current->is_dirty) {
            ssize_t bytes_written = pwrite(cache->fd, current->data, current->block_size, current->offset);

            if (bytes_written < 0) {
                perror("[!] Write block to disk while fsync");
                return -1;
            }

            current->is_dirty = false;
        }

        current = current->next_block;
    }

    return fsync(cache->fd);
}


ssize_t lab2_write(file_cache *cache, const void *buf, ssize_t count, off_t offset) {
    if (!cache) {
        perror("[!] Get cache");
        return -1;
    }

    cache_block *current = cache->head;

    while (current) {
        if (current->offset == offset) {
            memcpy(current->data, buf, count);

            current->is_dirty = true;
            current->usage_frequency++;
            return count;
        }

        current = current->next_block;
    }

    if (create_cache_block(cache, buf, count, offset) < 0) {
        return -1;
    }

    if (cache->current_blocks > cache->max_blocks) {
        lfu_cache_evict(cache);
    }

    return count;
}


ssize_t lab2_read(file_cache *cache, void *buf, ssize_t count, off_t offset) {
    if (!cache) {
        perror("[!] Get cache");
        return -1;
    }

    cache_block *current = cache->head;

    while (current) {
        if (current->offset == offset) {
            memcpy(buf, current->data, count);

            current->usage_frequency++;
            return count;
        }

        current = current->next_block;
    }

    create_cache_block(cache, buf, count, offset);


    const ssize_t bytes_read = pread(cache->fd, buf, count, offset);

    if (bytes_read < 0) {
        perror("[!] Read block from disk");
        free(buf);
        return -1;
    }

    memcpy(buf, current->data, count);
    return bytes_read;
}


off_t lab2_lseek(file_cache *cache, const off_t offset, const int whence) {
    if (!cache) {
        perror("[!] Get cache");
        return -1;
    }

    if (whence != SEEK_SET) {
        perror("[!] Seek mode");
        return -1;
    }

    const off_t new_offset = lseek(cache->fd, offset, whence);
    if (new_offset == (off_t)-1) {
        perror("[!] Reposition file offset");
        return -1;
    }

    cache->current_offset = new_offset;
    return new_offset;
}
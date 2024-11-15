#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>

// int lab2_open(const char *path);
// int lab2_close(int fd);
// ssize_t lab2_read(int fd, void *buf, size_t count);
// ssize_t lab2_write(int fd, const void *buf, size_t count);
// off_t lab2_lseek(int fd, off_t offset, int whence);
// int lab2_fsync(int fd);

typedef struct cache_block cache_block;
typedef struct file_cache file_cache;

struct cache_block {
    off_t offset;
    void *data;
    size_t block_size;
    int usage_frequency;
    bool dirty;
    cache_block *next_block;
};

struct file_cache {
    int fd;
    size_t block_size;
    size_t max_blocks;
    size_t current_blocks;
    cache_block *head;
};


int main(void) {
    printf("Hello, World!\n");
    return 0;
}

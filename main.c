#include "utils.h"
#include "api.h"

#define BLOCK_SIZE 4096
#define MAX_BLOCKS 4


int main() {
    const char *file_path = "../test.txt";

    file_cache *cache = lab2_open(file_path, BLOCK_SIZE, MAX_BLOCKS);
    if (!cache) {
        fprintf(stderr, "[!] Init cache\n");
        return EXIT_FAILURE;
    }

    printf("[+] Init cache\n");


    const char *write_data = "Hello, World!";
    const ssize_t data_len = (ssize_t) strlen(write_data);

    if (lab2_write(cache, write_data, data_len, 0) < 0) {
        fprintf(stderr, "[!] Write data\n");
        lab2_close(cache);

        return EXIT_FAILURE;
    }

    printf("[+] Write data\n");
    printf("Data written: %s\n", write_data);


    if (lab2_fsync(cache) < 0) {
        fprintf(stderr, "[!] Synchronize cache with disk\n");
        lab2_close(cache);

        return EXIT_FAILURE;
    }

    printf("[+] Synchronize data with disk\n");


    char read_data[1024];
    memset(read_data, 0, sizeof(read_data));

    if (lab2_read(cache, read_data, data_len, 0) < 0) {
        fprintf(stderr, "[!] Read data\n");
        lab2_close(cache);
        return EXIT_FAILURE;
    }

    printf("[+] Read data\n");
    printf("Data read: %s\n", read_data);


    if (lab2_lseek(cache, 1024, SEEK_SET) < 0) {
        perror("[!] Seek to position");
        lab2_close(cache);
        return EXIT_FAILURE;
    }

    printf("[+] Seek to position\n");


    lab2_close(cache);
    printf("[+] Close cache\n");

    return EXIT_SUCCESS;
}

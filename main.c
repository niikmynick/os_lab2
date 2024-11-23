#include <time.h>

#include "utils.h"
#include "api.h"
#include "lru.h"

#define BLOCK_SIZE 4096
#define MAX_BLOCKS 4


int example() {
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

int bin_search(const char *filename, int target) {
    FILE *file = fopen(filename, "rb");
    if (!file) {
        perror("[!] Open file for reading");
        return -1;
    }

    int left = 0, right = -1;
    fseek(file, 0, SEEK_END);
    right = ftell(file) / sizeof(int) - 1;

    int found = 0;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        int value;
        fseek(file, mid * sizeof(int), SEEK_SET);
        fread(&value, sizeof(int), 1, file);

        if (value == target) {
            printf("Found %d at position %d.\n", target, mid);
            found = 1;
            break;
        } else if (value < target) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    if (!found) {
        printf("Element %d not found in file.\n", target);
    }

    fclose(file);

    return 0;
}

int bin_search_lfu_cache_impl(const char *filename, int target) {
    file_cache *cache = lab2_open(filename, BLOCK_SIZE, MAX_BLOCKS);
    if (!cache) {
        perror("[!] Open file for reading");
        return -1;
    }

    int left = 0, right = -1;

    off_t file_size = lseek(cache->fd, 0, SEEK_END);
    if (file_size < 0) {
        perror("[!] Get file size");
        lab2_close(cache);
        return -1;
    }
    right = file_size / sizeof(int) - 1;

    int found = 0;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        int value;

        ssize_t bytes_read = lab2_read(cache, &value, sizeof(int), mid * sizeof(int));

        if (bytes_read < sizeof(int)) {
            perror("[!] Read value from cache");
            break;
        }

        if (value == target) {
            printf("Found %d at position %d.\n", target, mid);
            found = 1;
            break;
        } else if (value < target) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    if (!found) {
        printf("Element %d not found in file.\n", target);
    }

    lab2_close(cache);
    return 0;
}

int bin_search_lru_cache_impl(const char *filename, int target) {
    file_cache *cache = open_file(filename, BLOCK_SIZE, MAX_BLOCKS);
    if (!cache) {
        perror("[!] Open file for reading");
        return -1;
    }

    int left = 0, right = -1;

    off_t file_size = lseek(cache->fd, 0, SEEK_END);
    if (file_size < 0) {
        perror("[!] Get file size");
        close_file(cache);
        return -1;
    }
    right = file_size / sizeof(int) - 1;

    int found = 0;
    while (left <= right) {
        int mid = left + (right - left) / 2;
        int value;

        ssize_t bytes_read = read_file(cache, &value, sizeof(int), mid * sizeof(int));

        if (bytes_read < sizeof(int)) {
            perror("[!] Read value from cache");
            break;
        }

        if (value == target) {
            printf("Found %d at position %d.\n", target, mid);
            found = 1;
            break;
        } else if (value < target) {
            left = mid + 1;
        } else {
            right = mid - 1;
        }
    }

    if (!found) {
        printf("Element %d not found in file.\n", target);
    }

    close_file(cache);
    return 0;
}


int main() {
    // bin_search("../sorted.txt", 52);
    // bin_search_cache_impl("../sorted.txt", 52);

    clock_t start_time = clock();
    bin_search("../data.txt", 52);
    clock_t end_time = clock();
    printf("bin search: %f\n", (double)(end_time - start_time) / CLOCKS_PER_SEC);

    start_time = clock();
    bin_search_lfu_cache_impl("../data.txt", 52);
    end_time = clock();
    printf("bin search with LFU cache: %f\n", (double)(end_time - start_time) / CLOCKS_PER_SEC);

    start_time = clock();
    bin_search_lru_cache_impl("../data.txt", 52);
    end_time = clock();
    printf("bin search with LRU cache: %f\n", (double)(end_time - start_time) / CLOCKS_PER_SEC);

    return 0;
}

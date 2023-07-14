#include <stdio.h>
#include <stdbool.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include <string.h>
#include "pagedir.h"

bool pagedir_init(const char *pageDirectory) {
    struct stat st;
    if (stat(pageDirectory, &st) != 0) {
        if (mkdir(pageDirectory, 0777) != 0) {
            fprintf(stderr, "Failed to create page directory: %s\n", pageDirectory);
            return false;
        }
    }

    return true;
}

void pagedir_save(const webpage_t *page, const char *pageDirectory, const int documentID) {
    char filepath[512];
    snprintf(filepath, sizeof(filepath), "%s/%d", pageDirectory, documentID);
    //this will make the filepath with pagedirectory as well as documentid
    FILE *file = fopen(filepath, "w");
    //opening the file in order to write
    if (file == NULL) {
        fprintf(stderr, "Unable to open file: %s\n", filepath);
        //prints message in the case the file was not opened sucessfulya
        return;
    }
    fprintf(file, "%s\n%d\n%s\n", page->url, page->depth, page->html);
    fclose(file);
    //closing the file here
}

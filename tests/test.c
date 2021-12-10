#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <stdio.h>

int main() {
    char path[] = "/home/directory/path";
    char* parent_path = malloc(strlen(path) + 1);
    strcpy(parent_path, path);
    char* last_slash = strrchr(parent_path, '/');
    *(last_slash) = '\0';
    char name[FILENAME_MAX];
    strcpy(name, last_slash+1);
    printf("%s | %s", parent_path, name);
}
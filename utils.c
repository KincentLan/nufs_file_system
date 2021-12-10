#include <stdio.h>
#include <string.h>

#include "blocks.h"

void get_parent_path(const char* path, char* buf) {
    strcpy(buf, path);
    char* last_slash = strrchr(buf, '/');
    *(last_slash + 1) = '\0';
}

void get_base_name(const char* path, char* buf) {
  if (strcmp(path, "/") == 0) {
      strcpy(buf, "~");
  }
  else {
    char* last_slash = (char*) strrchr(path, '/');
    strcpy(buf, last_slash+1);
  }
}
#include <string.h>
#include <stdlib.h>

char* dirname(char* path) {
    char* s;
    if ((s = strrchr(path, '/'))) {
        *s = 0;
        return path;
    }
    return NULL;
}

char* strcopy(const char* str) {
    char* ret;
    if ((ret = malloc(strlen(str) + 1))) {
        memcpy(ret, str, strlen(str) + 1);
    }
    return ret;
}

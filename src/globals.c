#include <string.h>
#include <stdbool.h>
#include "globals.h"


bool any(bool array[], int size) {
    bool res = false;
    for (int i = 0; i < size; i++) {
        if (array[i]) {
            res = true;
            break;
        }
    }
    return res;
}

bool all(bool array[], int size) {
    bool res = true;
    for (int i = 0; i < size; i++) {
        if (!array[i]) {
            res = false;
            break;
        }
    }
    return res;
}

bool startsWith(const char *str, const char *prefix) {
    return strncmp(str, prefix, strlen(prefix)) == 0;
}


const char version[] = "beta 0.4.1";

struct settings storedSettings = {0};

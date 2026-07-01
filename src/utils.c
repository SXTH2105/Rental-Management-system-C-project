#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "utils.h"

#ifdef _WIN32
#include <direct.h>
#else
#include <sys/stat.h>
#include <sys/types.h>
#endif

void utils_trim(char *str) {
    if (!str) return;
    int start = 0;
    while (str[start] == ' ' || str[start] == '\t' || str[start] == '\r' || str[start] == '\n') {
        start++;
    }
    if (start > 0) {
        int i = 0;
        while (str[start + i] != '\0') {
            str[i] = str[start + i];
            i++;
        }
        str[i] = '\0';
    }
    int len = 0;
    while (str[len] != '\0') {
        len++;
    }
    int end = len - 1;
    while (end >= 0 && (str[end] == ' ' || str[end] == '\t' || str[end] == '\r' || str[end] == '\n')) {
        str[end] = '\0';
        end--;
    }
}

int utils_str_to_int(const char *str) {
    if (!str) return 0;
    return atoi(str);
}

float utils_str_to_float(const char *str) {
    if (!str) return 0.0f;
    return (float)atof(str);
}

void utils_today(char out_date[20]) {
    time_t t = time(NULL);
    struct tm *tm_info = localtime(&t);
    if (tm_info) {
        strftime(out_date, 20, "%d/%m/%Y", tm_info);
    } else {
        strcpy(out_date, "01/01/2026");
    }
}

int utils_next_id(const int ids[], int count) {
    int max_id = 0;
    for (int i = 0; i < count; i++) {
        if (ids[i] > max_id) {
            max_id = ids[i];
        }
    }
    return max_id + 1;
}

int utils_file_exists(const char *path) {
    if (!path) return 0;
    FILE *f = fopen(path, "r");
    if (f) {
        fclose(f);
        return 1;
    }
    return 0;
}

int utils_ensure_dir(const char *path) {
    if (!path) return 0;
#ifdef _WIN32
    _mkdir(path);
#else
    mkdir(path, 0777);
#endif
    return 1;
}

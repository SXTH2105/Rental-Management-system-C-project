#include <stdio.h>
#include <string.h>
#include "auth.h"
#include "utils.h"

int auth_load_users(User users[], int *count) {
    if (!count) return 0;
    *count = 0;
    FILE *f = fopen("data/users.txt", "r");
    if (!f) return 0;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        utils_trim(line);
        if (strlen(line) == 0) continue;

        User u;
        memset(&u, 0, sizeof(User));
        if (sscanf(line, "%d|%[^|]|%[^|]|%d", &u.id, u.username, u.phone, &u.role) == 4) {
            utils_trim(u.username);
            utils_trim(u.phone);
            if (*count < MAX_USERS) {
                users[*count] = u;
                (*count)++;
            }
        }
    }
    fclose(f);
    return 1;
}

int auth_save_users(const User users[], int count) {
    FILE *f = fopen("data/users.txt", "w");
    if (!f) return 0;

    for (int i = 0; i < count; i++) {
        fprintf(f, "%d|%s|%s|%d\n", users[i].id, users[i].username, users[i].phone, users[i].role);
    }
    fclose(f);
    return 1;
}

int auth_login(const char *username, const char *phone, User *out_user) {
    User users[MAX_USERS];
    int count = 0;
    if (!auth_load_users(users, &count)) {
        return 0;
    }

    char clean_user[50];
    char clean_phone[20];
    strncpy(clean_user, username, sizeof(clean_user) - 1);
    clean_user[sizeof(clean_user) - 1] = '\0';
    strncpy(clean_phone, phone, sizeof(clean_phone) - 1);
    clean_phone[sizeof(clean_phone) - 1] = '\0';
    
    utils_trim(clean_user);
    utils_trim(clean_phone);

    for (int i = 0; i < count; i++) {
        if (strcmp(users[i].username, clean_user) == 0 && strcmp(users[i].phone, clean_phone) == 0) {
            if (out_user) {
                *out_user = users[i];
            }
            return 1;
        }
    }
    return 0;
}

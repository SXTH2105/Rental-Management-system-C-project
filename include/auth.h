#ifndef AUTH_H
#define AUTH_H

#define MAX_USERS 10
#define ROLE_LANDLORD 0
#define ROLE_GUEST    1

typedef struct {
    int  id;
    char username[50];
    char phone[20];
    int  role;    //0 = landlord 1 = guest
} User;

int auth_login(const char *username, const char *phone, User *out_user);
int auth_load_users(User users[], int *count);
int auth_save_users(const User users[], int count);

#endif

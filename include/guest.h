#ifndef GUEST_H
#define GUEST_H

#define MAX_GUESTS 50

typedef struct {
    int  id;
    char name[50];
    char phone[20];
    int  room_id;
    char check_in[20];   //DD/MM/YYYY
    char check_out[20];
} Guest;

int  guest_load(Guest guests[], int *count);
int  guest_save(const Guest guests[], int count);
void guest_add(Guest guests[], int *count, const Guest *g);
int  guest_find_by_id(const Guest guests[], int count, int id);
void guest_delete(Guest guests[], int *count, int id);

#endif

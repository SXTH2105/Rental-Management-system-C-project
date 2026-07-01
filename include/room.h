#ifndef ROOM_H
#define ROOM_H

#define MAX_ROOMS 50
#define ROOM_AVAILABLE   0
#define ROOM_OCCUPIED    1
#define ROOM_MAINTENANCE 2

typedef struct {
    int   id;
    char  name[50];
    float price;
    int   status;     // 0 = available 1 = occupied 2 = maintenance
} Room;

int  room_load(Room rooms[], int *count);
int  room_save(const Room rooms[], int count);
void room_add(Room rooms[], int *count, const Room *r);
int  room_find_by_id(const Room rooms[], int count, int id);
void room_delete(Room rooms[], int *count, int id);

#endif

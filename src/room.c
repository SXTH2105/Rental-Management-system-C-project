#include <stdio.h>
#include <string.h>
#include "room.h"
#include "utils.h"

int room_load(Room rooms[], int *count) {
    if (!count) return 0;
    *count = 0;
    FILE *f = fopen("data/rooms.txt", "r");
    if (!f) return 0;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        utils_trim(line);
        if (strlen(line) == 0) continue;

        Room r;
        memset(&r, 0, sizeof(Room));
        if (sscanf(line, "%d|%[^|]|%f|%d", &r.id, r.name, &r.price, &r.status) == 4) {
            utils_trim(r.name);
            if (*count < MAX_ROOMS) {
                rooms[*count] = r;
                (*count)++;
            }
        }
    }
    fclose(f);
    return 1;
}

int room_save(const Room rooms[], int count) {
    FILE *f = fopen("data/rooms.txt", "w");
    if (!f) return 0;

    for (int i = 0; i < count; i++) {
        fprintf(f, "%d|%s|%.2f|%d\n", rooms[i].id, rooms[i].name, rooms[i].price, rooms[i].status);
    }
    fclose(f);
    return 1;
}

void room_add(Room rooms[], int *count, const Room *r) {
    if (!count || !r || *count >= MAX_ROOMS) return;
    rooms[*count] = *r;
    (*count)++;
}

int room_find_by_id(const Room rooms[], int count, int id) {
    for (int i = 0; i < count; i++) {
        if (rooms[i].id == id) {
            return i;
        }
    }
    return -1;
}

void room_delete(Room rooms[], int *count, int id) {
    if (!count) return;
    int idx = room_find_by_id(rooms, *count, id);
    if (idx == -1) return;

    for (int i = idx; i < *count - 1; i++) {
        rooms[i] = rooms[i + 1];
    }
    (*count)--;
}

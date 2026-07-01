#include <stdio.h>
#include <string.h>
#include "guest.h"
#include "utils.h"

int guest_load(Guest guests[], int *count) {
    if (!count) return 0;
    *count = 0;
    FILE *f = fopen("data/guests.txt", "r");
    if (!f) return 0;

    char line[256];
    while (fgets(line, sizeof(line), f)) {
        utils_trim(line);
        if (strlen(line) == 0) continue;

        Guest g;
        memset(&g, 0, sizeof(Guest));
        
        // Simple scanning for fields. check_out might be empty or blank so we scan up to 6 fields.
        int parsed = sscanf(line, "%d|%[^|]|%[^|]|%d|%[^|]|%[^|]",  &g.id, g.name, g.phone, &g.room_id, g.check_in, g.check_out);
        if (parsed >= 4) {
            utils_trim(g.name);
            utils_trim(g.phone);
            utils_trim(g.check_in);
            utils_trim(g.check_out);
            if (*count < MAX_GUESTS) {
                guests[*count] = g;
                (*count)++;
            }
        }
    }
    fclose(f);
    return 1;
}

int guest_save(const Guest guests[], int count) {
    FILE *f = fopen("data/guests.txt", "w");
    if (!f) return 0;

    for (int i = 0; i < count; i++) {
        fprintf(f, "%d|%s|%s|%d|%s|%s\n", 
                guests[i].id, guests[i].name, guests[i].phone, 
                guests[i].room_id, guests[i].check_in, guests[i].check_out);
    }
    fclose(f);
    return 1;
}

void guest_add(Guest guests[], int *count, const Guest *g) {
    if (!count || !g || *count >= MAX_GUESTS) return;
    guests[*count] = *g;
    (*count)++;
}

int guest_find_by_id(const Guest guests[], int count, int id) {
    for (int i = 0; i < count; i++) {
        if (guests[i].id == id) {
            return i;
        }
    }
    return -1;
}

void guest_delete(Guest guests[], int *count, int id) {
    if (!count) return;
    int idx = guest_find_by_id(guests, *count, id);
    if (idx == -1) return;

    for (int i = idx; i < *count - 1; i++) {
        guests[i] = guests[i + 1];
    }
    (*count)--;
}

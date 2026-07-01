#include <stdio.h>
#include <string.h>
#include "report.h"
#include "utils.h"

int report_load(Report reports[], int *count) {
    if (!count) return 0;
    *count = 0;
    FILE *f = fopen("data/reports.txt", "r");
    if (!f) return 0;

    char line[512];
    while (fgets(line, sizeof(line), f)) {
        utils_trim(line);
        if (strlen(line) == 0) continue;

        Report r;
        memset(&r, 0, sizeof(Report));
        
        // Split and read. Date might be at the end, so we read 6 fields.
        int parsed = sscanf(line, "%d|%d|%d|%[^|]|%d|%[^|]", 
                            &r.id, &r.guest_id, &r.room_id, r.description, &r.status, r.date);
        if (parsed >= 5) {
            utils_trim(r.description);
            utils_trim(r.date);
            if (*count < MAX_REPORTS) {
                reports[*count] = r;
                (*count)++;
            }
        }
    }
    fclose(f);
    return 1;
}

int report_save(const Report reports[], int count) {
    FILE *f = fopen("data/reports.txt", "w");
    if (!f) return 0;

    for (int i = 0; i < count; i++) {
        fprintf(f, "%d|%d|%d|%s|%d|%s\n", 
                reports[i].id, reports[i].guest_id, reports[i].room_id, 
                reports[i].description, reports[i].status, reports[i].date);
    }
    fclose(f);
    return 1;
}

void report_add(Report reports[], int *count, const Report *r) {
    if (!count || !r || *count >= MAX_REPORTS) return;
    reports[*count] = *r;
    (*count)++;
}

void report_update_status(Report reports[], int count, int report_id, int new_status) {
    for (int i = 0; i < count; i++) {
        if (reports[i].id == report_id) {
            reports[i].status = new_status;
            break;
        }
    }
}

int report_find_by_guest(const Report reports[], int count, int guest_id) {
    for (int i = 0; i < count; i++) {
        if (reports[i].guest_id == guest_id) {
            return i;
        }
    }
    return -1;
}

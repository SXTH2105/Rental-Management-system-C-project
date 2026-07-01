#ifndef REPORT_H
#define REPORT_H

#define MAX_REPORTS 100
#define REPORT_PENDING     0
#define REPORT_IN_PROGRESS 1
#define REPORT_RESOLVED    2

typedef struct {
    int  id;
    int  guest_id;
    int  room_id;
    char description[200];
    int  status;     // 0 = pending 1 = in progress 2 = resolved
    char date[20];
} Report;

int  report_load(Report reports[], int *count);
int  report_save(const Report reports[], int count);
void report_add(Report reports[], int *count, const Report *r);
void report_update_status(Report reports[], int count, int report_id, int new_status);
int  report_find_by_guest(const Report reports[], int count, int guest_id);

#endif

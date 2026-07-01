#include <stdio.h>
#include <string.h>
#include "billing.h"
#include "utils.h"

int billing_load(Bill bills[], int *count) {
    if (!count) return 0;
    *count = 0;
    FILE *f = fopen("data/bills.txt", "r");
    if (!f) return 0;

    char line[512];
    while (fgets(line, sizeof(line), f)) {
        utils_trim(line);
        if (strlen(line) == 0) continue;

        Bill b;
        memset(&b, 0, sizeof(Bill));
        int n = sscanf(line,
            "%d|%d|%f|%f|%f|%f|%f|%f|%f|%f|%[^|]|%d",
            &b.id, &b.guest_id,
            &b.room_fee,
            &b.elec_prev, &b.elec_curr,
            &b.water_prev, &b.water_curr,
            &b.electric, &b.water,
            &b.total,
            b.due_date, &b.paid);

        if (n == 12) {
            utils_trim(b.due_date);
            if (*count < MAX_BILLS) bills[(*count)++] = b;
            continue;
        }
        memset(&b, 0, sizeof(Bill));
        n = sscanf(line,
            "%d|%d|%f|%f|%f|%f|%[^|]|%d",
            &b.id, &b.guest_id,
            &b.room_fee, &b.electric, &b.water, &b.total,
            b.due_date, &b.paid);

        if (n == 8) {
            utils_trim(b.due_date);
            if (*count < MAX_BILLS) bills[(*count)++] = b;
        }
    }
    fclose(f);
    return 1;
}

int billing_save(const Bill bills[], int count) {
    FILE *f = fopen("data/bills.txt", "w");
    if (!f) return 0;

    for (int i = 0; i < count; i++) {
        fprintf(f, "%d|%d|%.2f|%.2f|%.2f|%.2f|%.2f|%.2f|%.2f|%.2f|%s|%d\n",
                bills[i].id,
                bills[i].guest_id,
                bills[i].room_fee,
                bills[i].elec_prev,
                bills[i].elec_curr,
                bills[i].water_prev,
                bills[i].water_curr,
                bills[i].electric,
                bills[i].water,
                bills[i].total,
                bills[i].due_date,
                bills[i].paid);
    }
    fclose(f);
    return 1;
}

void billing_add(Bill bills[], int *count, const Bill *b) {
    if (!count || !b || *count >= MAX_BILLS) return;
    bills[*count] = *b;
    bills[*count].total = billing_calc_total(&bills[*count]);
    (*count)++;
}

float billing_calc_total(const Bill *b) {
    if (!b) return 0.0f;
    return b->room_fee + b->electric + b->water;
}

void billing_mark_paid(Bill bills[], int count, int bill_id) {
    for (int i = 0; i < count; i++) {
        if (bills[i].id == bill_id) {
            bills[i].paid = BILL_PAID;
            break;
        }
    }
}

int billing_find_by_guest(const Bill bills[], int count, int guest_id) {
    for (int i = 0; i < count; i++) {
        if (bills[i].guest_id == guest_id) {
            return i;
        }
    }
    return -1;
}

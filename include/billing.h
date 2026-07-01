#ifndef BILLING_H
#define BILLING_H
#define MAX_BILLS 100
#define BILL_UNPAID 0
#define BILL_PAID   1

typedef struct {
    int   id;
    int   guest_id;
    float room_fee;

    float elec_prev;
    float elec_curr;
    float water_prev;
    float water_curr;
    float electric;
    float water;

    float total;
    char  due_date[20];
    int   paid;
} Bill;

int   billing_load(Bill bills[], int *count);
int   billing_save(const Bill bills[], int count);
void  billing_add(Bill bills[], int *count, const Bill *b);
float billing_calc_total(const Bill *b);
void  billing_mark_paid(Bill bills[], int count, int bill_id);
int   billing_find_by_guest(const Bill bills[], int count, int guest_id);

#endif

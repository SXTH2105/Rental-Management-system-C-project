#include <stdio.h>
#include <string.h>
#include "invoice.h"
#include "utils.h"
#include "room.h"

Invoice invoice_build(const Guest *g, const Bill *b, const char *issued_date) {
    Invoice inv;
    memset(&inv, 0, sizeof(Invoice));
    if (b) {
        inv.invoice_id = b->id;
    } else {
        inv.invoice_id = 1;
    }
    if (g) inv.guest = *g;
    if (b) inv.bill = *b;
    if (issued_date) {
        strncpy(inv.issued_date, issued_date, sizeof(inv.issued_date) - 1);
    } else {
        utils_today(inv.issued_date);
    }
    return inv;
}

int invoice_save_txt(const Invoice *inv, const char *dir_path) {
    if (!inv || !dir_path) return 0;
    utils_ensure_dir(dir_path);

    char filepath[MAX_INVOICE_PATH];
    snprintf(filepath, sizeof(filepath), "%s/invoice_%d_%d.txt", dir_path, inv->guest.id, inv->bill.id);

    FILE *f = fopen(filepath, "w");
    if (!f) return 0;

    Room rooms[MAX_ROOMS];
    int room_count = 0;
    room_load(rooms, &room_count);
    char room_name[50] = "Unknown Room";
    int idx = room_find_by_id(rooms, room_count, inv->guest.room_id);
    if (idx != -1) {
        strncpy(room_name, rooms[idx].name, sizeof(room_name) - 1);
    }

    fprintf(f, "================================\n");
    fprintf(f, "      RENTAL INVOICE\n");
    fprintf(f, "================================\n");
    fprintf(f, "Invoice ID : %d\n", inv->invoice_id);
    fprintf(f, "Date       : %s\n", inv->issued_date);
    fprintf(f, "Guest Name : %s\n", inv->guest.name);
    fprintf(f, "Phone      : %s\n", inv->guest.phone);
    fprintf(f, "Room       : %s\n", room_name);
    fprintf(f, "Check-in   : %s\n", inv->guest.check_in);
    fprintf(f, "--------------------------------\n");
    fprintf(f, "Room Fee   : %.2f\n", inv->bill.room_fee);
    fprintf(f, "Electric   : %.2f\n", inv->bill.electric);
    fprintf(f, "Water      : %.2f\n", inv->bill.water);
    fprintf(f, "--------------------------------\n");
    fprintf(f, "TOTAL      : %.2f\n", inv->bill.total);
    fprintf(f, "Due Date   : %s\n", inv->bill.due_date);
    fprintf(f, "Status     : %s\n", inv->bill.paid ? "Paid" : "Unpaid");
    fprintf(f, "================================\n");

    fclose(f);
    return 1;
}

void invoice_preview(const Invoice *inv) {
    if (!inv) return;
    Room rooms[MAX_ROOMS];
    int room_count = 0;
    room_load(rooms, &room_count);
    char room_name[50] = "Unknown Room";
    int idx = room_find_by_id(rooms, room_count, inv->guest.room_id);
    if (idx != -1) {
        strncpy(room_name, rooms[idx].name, sizeof(room_name) - 1);
    }

    printf("================================\n");
    printf("      RENTAL INVOICE\n");
    printf("================================\n");
    printf("Invoice ID : %d\n", inv->invoice_id);
    printf("Date       : %s\n", inv->issued_date);
    printf("Guest Name : %s\n", inv->guest.name);
    printf("Phone      : %s\n", inv->guest.phone);
    printf("Room       : %s\n", room_name);
    printf("Check-in   : %s\n", inv->guest.check_in);
    printf("--------------------------------\n");
    printf("Room Fee   : %.2f\n", inv->bill.room_fee);
    printf("Electric   : %.2f\n", inv->bill.electric);
    printf("Water      : %.2f\n", inv->bill.water);
    printf("--------------------------------\n");
    printf("TOTAL      : %.2f\n", inv->bill.total);
    printf("Due Date   : %s\n", inv->bill.due_date);
    printf("Status     : %s\n", inv->bill.paid ? "Paid" : "Unpaid");
    printf("================================\n");
}


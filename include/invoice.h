#ifndef INVOICE_H
#define INVOICE_H

#include "guest.h"
#include "billing.h"

#define MAX_INVOICE_PATH 256

typedef struct {
    int   invoice_id;
    Guest guest;
    Bill  bill;
    char  issued_date[20];  //DD/MM/YYYY
} Invoice;

Invoice invoice_build(const Guest *g, const Bill *b, const char *issued_date);
int     invoice_save_txt(const Invoice *inv, const char *dir_path);
void    invoice_preview(const Invoice *inv);

#endif

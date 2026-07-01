#include <stdio.h>
#include "rates.h"

int rates_load(Rates *r) {
    if (!r) return 0;
    r->elec_rate  = DEFAULT_ELEC_RATE;
    r->water_rate = DEFAULT_WATER_RATE;

    FILE *f = fopen("data/rates.txt", "r");
    if (!f) return 0;
    fscanf(f, "%f|%f", &r->elec_rate, &r->water_rate);
    fclose(f);
    return 1;
}

int rates_save(const Rates *r) {
    if (!r) return 0;
    FILE *f = fopen("data/rates.txt", "w");
    if (!f) return 0;
    fprintf(f, "%.4f|%.4f\n", r->elec_rate, r->water_rate);
    fclose(f);
    return 1;
}

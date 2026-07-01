#ifndef RATES_H
#define RATES_H


#define DEFAULT_ELEC_RATE  0.15f
#define DEFAULT_WATER_RATE 0.50f

typedef struct {
    float elec_rate;
    float water_rate;
} Rates;

int   rates_load(Rates *r);
int   rates_save(const Rates *r);

#endif

#ifndef ADC_AIM_H
#define ADC_AIM_H

#include <stdbool.h>

void adc_aim_init(void);
void adc_aim_update(int *x, int *y);
bool adc_aim_button_pressed(void);

#endif

#ifndef _HX_READER_H_
#define _HX_READER_H_

void dataReadyISR();
void hx_reader_setup();
void hx_reader_loop();
void hx_tara();
double get_force();
bool contact_detected();

double get_filtered_force();

#endif
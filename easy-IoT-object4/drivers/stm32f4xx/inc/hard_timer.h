#ifndef _HARD_TIMER_H
#define _HARD_TIMER_H

void start_timer(uint32_t freq, void *call_back);
void pause_timer(void);
void continue_timer(void);
uint32_t get_hard_timer_clk(void);
void mode_timer_by_counter(uint32_t counter);
#endif

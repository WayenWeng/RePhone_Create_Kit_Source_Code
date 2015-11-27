
#ifndef _IFTTT_BOOK_H
#define _IFTTT_BOOK_H


#define IF_NAME_MAX     10
#define THEN_NAME_MAX	5
#define IF_SHORT_NAME_MAX     	8
#define THEN_SHORT_NAME_MAX		7


extern void led_matrix_do_action(unsigned long *pdata);
extern void rgb_ws2812_do_action(unsigned long *pdata);
extern void music_do_action(unsigned long *pdata);
extern void motor_do_action(unsigned long *pdata);
extern void call_do_action(unsigned long *pdata);
extern void sms_do_action(unsigned long *pdata);


void ifttt_book_open();
void ifttt_book_save();
void ifttt_book_show();
void ifttt_book_add(char *content);


#endif

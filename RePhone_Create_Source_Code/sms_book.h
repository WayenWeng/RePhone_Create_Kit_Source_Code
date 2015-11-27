
#ifndef _SMS_BOOK_H
#define _SMS_BOOK_H


#define ADDRESS_MAX     20
#define CONTENT_LEN_MAX     100


void sms_book_open();
void sms_book_add(char* content);
int sms_book_find(char* content);
void sms_book_delete(char* content);
void sms_save();
int  sms_get_max_item_num();
const char* sms_get_item(unsigned int num);
void sms_book_show();


#endif

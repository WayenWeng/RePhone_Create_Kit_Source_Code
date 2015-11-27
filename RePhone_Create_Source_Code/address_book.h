
#ifndef _ADDRESS_BOOK_H
#define _ADDRESS_BOOK_H


#define ADDRESS_MAX     20
#define NAME_LEN_MAX    24
#define NUM_LEN_MAX     16


void book_open();                               // get data from flash
void book_add(char* name, char* num);
int  book_find(char* name);                      // return item num, find nothing return -1
void book_delete(char* name);                    // delete a certain item
void book_show();
void book_save();
int  book_get_number();
void book_get_item(unsigned int num, char *buf, unsigned int len);
const char* book_get_item_name(unsigned int num);
const char* book_get_item_number(unsigned int num);


#endif

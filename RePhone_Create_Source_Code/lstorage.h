
#ifndef _LSTORAGE_H
#define _LSTORAGE_H


void file_create(const char* fileName);
void file_open(const char* fileName);
void file_write(const char* fileName, const char* strBuf, long pos);
void file_delete(const char* fileName);
void file_read(const char* fileName, char* strBuf, unsigned int nByte, long pos);
void file_size(const char* fileName, unsigned long* size);


#endif

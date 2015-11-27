
#include "vmlog.h"
#include "sms_book.h"
#include "lstorage.h"
#include <stdio.h>
#include <string.h>


char sms_content[ADDRESS_MAX][CONTENT_LEN_MAX];
int  item_num;


void sms_book_open()
{
	unsigned long len;
	unsigned long i;
	item_num = 0;

	file_open("sms_book.txt");
	file_size("sms_book.txt", &len);

	if(len > 2)
	{
		char str_tmp[ADDRESS_MAX*(CONTENT_LEN_MAX+5)];
		file_read("sms_book.txt", str_tmp, len, 0);

		for(i=0; i<len; )
		{
			sscanf(str_tmp + i,"%[^\r\n]", sms_content[item_num]);

			i += strlen(sms_content[item_num]) + 2;
			item_num ++;

			if(item_num >= ADDRESS_MAX)item_num = ADDRESS_MAX;
		}
	}
}

void sms_book_add(char* content)
{
	if(content == 0)return;
	if(item_num == ADDRESS_MAX)return;

	if(item_num == 0)
	{
		strcpy(sms_content[0], content);

		item_num++;
		sms_save();

		return;
	}

	int is_exist = sms_book_find(content);

	// if this item exist
	if(is_exist>=0)
	{
		return;
	}

	char str_dbg[100];
	int min = 0;
	int max = item_num;

	while(min != max)
	{
		int tmp = (max+min)/2;

		min = strcmp(content, sms_content[tmp])>0 ? tmp+1 : min;
		max = strcmp(content, sms_content[tmp])<0 ? tmp : max;
	}

	int i;
	for(i=item_num; i>min; i--)
	{
		strcpy(sms_content[i], sms_content[i-1]);
	}

	strcpy(sms_content[min], content);

	item_num++;
	sms_save();
}

int sms_book_find(char* content)
{
	int i;

    if(content == 0)return -2;

    for(i=0; i<item_num; i++)
    {
        if(strcmp(content, sms_content[i]) == 0)return i;
    }
    return -1;
}

void sms_book_delete(char* content)
{
	int i;
	int is_exist;

    if(content == 0)return;
    if(item_num == 0)return;

    is_exist = sms_book_find(content);
    if(is_exist<0)return;

    for(i=is_exist; i<item_num-1; i++)
    {
        strcpy(sms_content[i], sms_content[i+1]);
    }

    item_num--;
    sms_save();
}

void sms_save()
{
	int i;

	file_delete("sms_book.txt");
	file_open("sms_book.txt");

	char str[100];
	for(i=0;i<item_num;i++)
	{
		sprintf(str, "%s\r\n", sms_content[i]);
		file_write("sms_book.txt", str, 0);
	}
}

int  sms_get_max_item_num()
{
	return item_num;
}

const char* sms_get_item(unsigned int num)
{
	return sms_content[num];
}

void sms_book_show()
{
	int i;
    for(i=0; i<item_num; i++)
    {
        vm_log_info("sms content: %s", sms_content[i]);
    }
}

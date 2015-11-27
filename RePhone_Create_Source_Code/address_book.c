
#include "vmlog.h"
#include "address_book.h"
#include "lstorage.h"
#include <stdio.h>
#include <string.h>


char book_name[ADDRESS_MAX][NAME_LEN_MAX];
char book_num[ADDRESS_MAX][NUM_LEN_MAX];
static int  g_book_item_number;


void book_open()
{
	unsigned long len;
	unsigned long i;
	g_book_item_number = 0;

	file_open("address_book.txt");
	file_size("address_book.txt", &len);

	if(len > 4)
	{
		char str_tmp[ADDRESS_MAX*(NAME_LEN_MAX+NUM_LEN_MAX+5)];
		file_read("address_book.txt", str_tmp, len, 0);

		for(i=0; i<len; )
		{
			sscanf(str_tmp + i,"%[^,]", book_name[g_book_item_number]);
			sscanf(str_tmp + i,"%*[^,],%[^\r\n]", book_num[g_book_item_number]);

			i += strlen(book_name[g_book_item_number]) + strlen(book_num[g_book_item_number]) + 3;
			g_book_item_number ++;

			if(g_book_item_number >= ADDRESS_MAX)g_book_item_number = ADDRESS_MAX;
		}
	}
}

void book_add(char* name, char* num)
{
	if(name == 0 || num == 0)return;
	if(g_book_item_number == ADDRESS_MAX)return;

	if(g_book_item_number == 0)
	{
		strcpy(book_name[0], name);
		strcpy(book_num[0], num);

		g_book_item_number++;
		book_save();

		return;
	}

	int is_exist = book_find(name);

	// if this item exist
	if(is_exist>=0)
	{
		strcpy(book_num[is_exist], num);
		book_save();

		return;
	}

	char str_dbg[50];
	int min = 0;
	int max = g_book_item_number;

	while(min != max)
	{
		int tmp = (max+min)/2;

		min = strcmp(name, book_name[tmp])>0 ? tmp+1 : min;
		max = strcmp(name, book_name[tmp])<0 ? tmp : max;
	}

	int i;
	for(i=g_book_item_number; i>min; i--)
	{
		strcpy(book_name[i], book_name[i-1]);
		strcpy(book_num[i], book_num[i-1]);
	}

	strcpy(book_num[min], num);
	strcpy(book_name[min], name);

	g_book_item_number++;
	book_save();
}

int book_find(char* name)
{
	int i;

    if(name == 0)return -2;

    for(i=0; i<g_book_item_number; i++)
    {
        if(strcmp(name, book_name[i]) == 0)return i;
    }
    return -1;
}

void book_delete(char* name)
{
	int i;
	int is_exist;

    if(name == 0)return;
    if(g_book_item_number == 0)return;

    is_exist = book_find(name);
    if(is_exist<0)return;

    for(i=is_exist; i<g_book_item_number-1; i++)
    {
        strcpy(book_name[i], book_name[i+1]);
        strcpy(book_num[i], book_num[i+1]);
    }

    g_book_item_number--;
    book_save();
}

void book_show()
{
	int i;
    for(i=0; i<g_book_item_number; i++)
    {
        vm_log_info("name: %s , number: %s", book_name[i], book_num[i]);
    }
}

void book_save()
{
	int i;

	file_delete("address_book.txt");
	file_open("address_book.txt");

	char str[50];
	for(i=0;i<g_book_item_number;i++)
	{
		snprintf(str, sizeof(str), "%s,%s\r\n", book_name[i], book_num[i]);
		file_write("address_book.txt", str, 0);
	}

}

int book_get_number()
{
	return g_book_item_number;
}

void book_get_item(unsigned int num, char *buf, unsigned int len)
{
    snprintf(buf, len, "%s - %s", book_name[num], book_num[num]);
}

const char* book_get_item_name(unsigned int num)
{
	return book_name[num];
}

const char* book_get_item_number(unsigned int num)
{
	return book_num[num];
}

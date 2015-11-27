
#include "vmtype.h"
#include "vmlog.h"
#include "ifttt_book.h"
#include "lstorage.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "cjson.h"
#include "actuator.h"
#include "condition.h"
#include "action.h"
#include "ifttt.h"
#include "laudio.h"


static const char if_name[IF_NAME_MAX][16] = {"acc.x", "acc.y", "acc.z", "light", "temperature", "humidity", "acc", "call", "sms", "button"};
static const char then_name[THEN_NAME_MAX][16] = {"led matrix", "color pixels", "music", "call", "sms"};

static unsigned char g_condition_number = 0;
static unsigned char g_action_number = 0;


extern actuator_pfunc_t g_actuator_do_action_function_list[];
extern condition_t g_condition_list[CONDITION_MAX_NUMBER];
extern uint32_t g_condition_mask;
extern action_pfunc_t g_action_pfunc_list[ACTION_MAX_NUMBER];
extern uint32_t g_action_data_table[ACTION_MAX_NUMBER][3];
extern uint32_t g_action_mask;
extern ifttt_t g_ifttt_list[IFTTT_MAX_NUMBER];
extern ifttt_t *g_ifttt_last;
extern int g_ifttt_number;


void ifttt_book_open()
{
	unsigned long len;
	file_open("ifttt_book.txt");
	file_size("ifttt_book.txt", &len);

	unsigned char* p;
	p = (unsigned char*)vm_malloc(len);
	if(p == NULL)
	{
		vm_log_info("vm_malloc is null");
		return;
	}
	file_read("ifttt_book.txt", p, len, 0);

	cJSON * pJson = cJSON_Parse(p);
	if(NULL == pJson)
	{
		vm_log_info("parse fail");
		vm_free(p);
		return ;
	}

	g_condition_number = 0;
	g_action_number = 0;
	g_condition_mask = 0;
	g_action_mask = 0;
	g_ifttt_number = 0;

	int RootArrayNum = cJSON_GetArraySize(pJson);
	vm_log_info("Root array number is %d", RootArrayNum);

	int i;
	unsigned char condition_number;
	unsigned char action_number;

	for(i=0;i<RootArrayNum;i++)
	{
		condition_number = 0;
		action_number = 0;
		cJSON * pIfttt = cJSON_GetArrayItem(pJson, i);
		if(NULL != pIfttt)
		{
			cJSON * pCondition = cJSON_GetObjectItem(pIfttt, "condition");
			if(NULL != pCondition)
			{
				int ConditonNum = cJSON_GetArraySize(pCondition);
				//vm_log_info("Condition array%d number is %d", i, ConditonNum);
				int j;
				for(j=0;j<ConditonNum;j++)
				{
					cJSON * pConObj = cJSON_GetArrayItem(pCondition, j);
					if(NULL != pConObj)
					{
						if(g_condition_number < CONDITION_MAX_NUMBER)
						{
							cJSON * pConObjItem = cJSON_GetObjectItem(pConObj, "if");
							if(NULL != pConObjItem)
							{
								//vm_log_info("if:%s", pConObjItem->valuestring);
								int m;
								for(m=0;m<IF_NAME_MAX;m++)
								{
									if(strcmp(if_name[m], pConObjItem->valuestring) == 0)
									{
										g_condition_list[g_condition_number].id = m; // id

										switch(m) // type
										{
											case 3: // light
											case 9: // button
												g_condition_list[g_condition_number].type = 0; // u32

												pConObjItem = cJSON_GetObjectItem(pConObj, "value"); // value
												if(NULL != pConObjItem)
												{
													//vm_log_info("value %.0f", pConObjItem->valuedouble);
													g_condition_list[g_condition_number].u32 = pConObjItem->valuedouble;
												}
											break;

											case 0: // acc.x
											case 1:	// acc.y
											case 2: // acc.z
											case 4: // temperature
												g_condition_list[g_condition_number].type = 1; // i32

												pConObjItem = cJSON_GetObjectItem(pConObj, "value"); // value
												if(NULL != pConObjItem)
												{
													//vm_log_info("value %.0f", pConObjItem->valuedouble);
													g_condition_list[g_condition_number].i32 = pConObjItem->valuedouble;
												}
											break;

											case 7: // call
												g_condition_list[g_condition_number].type = 16; // void*
												pConObjItem = cJSON_GetObjectItem(pConObj, "number"); // number
												if(NULL != pConObjItem)
												{
													g_condition_list[g_condition_number].p = (unsigned long)vm_malloc(16);
													strcpy(g_condition_list[g_condition_number].p, pConObjItem->valuestring);
												}
											break;

											case 8: // sms
												g_condition_list[g_condition_number].type = 16; // void*
												pConObjItem = cJSON_GetObjectItem(pConObj, "number"); // number
												if(NULL != pConObjItem)
												{
													g_condition_list[g_condition_number].p = (unsigned long)vm_malloc(16);
													strcpy(g_condition_list[g_condition_number].p, pConObjItem->valuestring);
												}
											break;

											default:
											break;
										}

										pConObjItem = cJSON_GetObjectItem(pConObj, "operator"); // operator
										if(NULL != pConObjItem)
										{
											//vm_log_info("operator %s", pConObjItem->valuestring);
											if(strcmp(pConObjItem->valuestring,">") == 0)g_condition_list[g_condition_number].op = '>';
											else if(strcmp(pConObjItem->valuestring,"=") == 0)g_condition_list[g_condition_number].op = '=';
											else if(strcmp(pConObjItem->valuestring,"<") == 0)g_condition_list[g_condition_number].op = '<';
											else g_condition_list[g_condition_number].op = 0;
										}

										condition_number ++;
										g_condition_number ++;
										break;
									}
								}
							}
						}
					}
				}
			}
			cJSON * pAction = cJSON_GetObjectItem(pIfttt, "action");
			if(NULL != pAction)
			{
				int ActionNum = cJSON_GetArraySize(pAction);
				//vm_log_info("Action array%d number is %d", i, ActionNum);
				int k;
				for(k=0;k<ActionNum;k++)
				{
					cJSON * pActObj = cJSON_GetArrayItem(pAction, k);
					if(NULL != pActObj)
					{
						if(g_action_number < ACTION_MAX_NUMBER)
						{
							cJSON * pActObjItem = cJSON_GetObjectItem(pActObj, "then");
							if(NULL != pActObjItem)
							{
								//vm_log_info("then:%s", pActObjItem->valuestring);
								int n;
								for(n=0;n<THEN_NAME_MAX;n++)
								{
									if(strcmp(then_name[n], pActObjItem->valuestring) == 0)
									{
										g_action_pfunc_list[g_action_number] = g_actuator_do_action_function_list[n];

										char* temStr;

										switch(n)
										{
											case 0: // led_matrix
												pActObjItem = cJSON_GetObjectItem(pActObj, "display");
												if(NULL != pActObjItem)
												{
													//vm_log_info("display %s", pActObjItem->valuestring);
													g_action_data_table[g_action_number][0] = pActObjItem->valuestring[0];
												}
												pActObjItem = cJSON_GetObjectItem(pActObj, "time");
												if(NULL != pActObjItem)
												{
													//vm_log_info("time %d", pActObjItem->valuedouble);
													g_action_data_table[g_action_number][1] = pActObjItem->valuedouble;
													action_number ++;
													g_action_number ++;
												}
											break;

											case 1: // rgb_ws2812
												pActObjItem = cJSON_GetObjectItem(pActObj, "mode");
												if(NULL != pActObjItem)
												{
													//vm_log_info("mode %s", pActObjItem->valuestring);
													if(strcmp("monochrome", pActObjItem->valuestring) == 0)
													{
														g_action_data_table[g_action_number][0] = 0;
														g_action_data_table[g_action_number][0] <<= 16;
													}
													else if(strcmp("marquee", pActObjItem->valuestring) == 0)
													{
														g_action_data_table[g_action_number][0] = 1;
														g_action_data_table[g_action_number][0] <<= 16;
													}
													else if(strcmp("rainbow", pActObjItem->valuestring) == 0)
													{
														g_action_data_table[g_action_number][0]= 2;
														g_action_data_table[g_action_number][0] <<= 16;
													}
												}

												pActObjItem = cJSON_GetObjectItem(pActObj, "led count");
												if(NULL != pActObjItem)
												{
													//vm_log_info("number %.0f", pActObjItem->valuedouble);
													g_action_data_table[g_action_number][0] += pActObjItem->valuedouble;
												}

												pActObjItem = cJSON_GetObjectItem(pActObj, "time");
												if(NULL != pActObjItem)
												{
													//vm_log_info("time %.0f", pActObjItem->valuedouble);
													g_action_data_table[g_action_number][1] = pActObjItem->valuedouble;

													if((g_action_data_table[g_action_number][0] >> 16) == 2)
													{
														action_number ++;
														g_action_number ++;
													}
												}

												if((g_action_data_table[g_action_number][0] >> 16) < 2)
												{

													pActObjItem = cJSON_GetObjectItem(pActObj, "color r");
													if(NULL != pActObjItem)
													{
														//vm_log_info("color_r %.0f", pActObjItem->valuedouble);
														g_action_data_table[g_action_number][2] = pActObjItem->valuedouble;
														g_action_data_table[g_action_number][2] <<= 8;
													}

													pActObjItem = cJSON_GetObjectItem(pActObj, "color g");
													if(NULL != pActObjItem)
													{
														//vm_log_info("color_g %.0f", pActObjItem->valuedouble);
														g_action_data_table[g_action_number][2] += pActObjItem->valuedouble;
														g_action_data_table[g_action_number][2] <<= 8;
													}

													pActObjItem = cJSON_GetObjectItem(pActObj, "color b");
													if(NULL != pActObjItem)
													{
														//vm_log_info("color_b %.0f", pActObjItem->valuedouble);
														g_action_data_table[g_action_number][2] += pActObjItem->valuedouble;
														action_number ++;
														g_action_number ++;
													}
												}
											break;

											case 2: // music
												pActObjItem = cJSON_GetObjectItem(pActObj, "number");
												if(NULL != pActObjItem)
												{
													//vm_log_info("number %d", pActObjItem->valuedouble);
													g_action_data_table[g_action_number][0] = pActObjItem->valuedouble;;
													action_number ++;
													g_action_number ++;
												}
											break;

											case 3: // call
												pActObjItem = cJSON_GetObjectItem(pActObj, "number");
												if(NULL != pActObjItem)
												{
													//vm_log_info("number %.0f", pActObjItem->valuestring);
													g_action_data_table[g_action_number][0] = (unsigned long)vm_malloc(16);
													strcpy(g_action_data_table[g_action_number][0], pActObjItem->valuestring);
													action_number ++;
													g_action_number ++;
												}
											break;

											case 4: // sms
												pActObjItem = cJSON_GetObjectItem(pActObj, "number");
												if(NULL != pActObjItem)
												{
													g_action_data_table[g_action_number][0] = (unsigned long)vm_malloc(56);
													temStr = g_action_data_table[g_action_number][0];
													strcpy(temStr, pActObjItem->valuestring);
												}

												pActObjItem = cJSON_GetObjectItem(pActObj, "content");
												if(NULL != pActObjItem)
												{
													strcpy(temStr + 16, pActObjItem->valuestring);
													action_number ++;
													g_action_number ++;
												}
											break;

											default:
											break;
										}
										break;
									}
								}
							}
						}
					}
				}
			}
		}

		if(g_ifttt_number < IFTTT_MAX_NUMBER)
		{
			int l;
			g_ifttt_list[g_ifttt_number].condition_mask = g_condition_mask;
			for(l=0;l<condition_number;l++)
			{
				g_ifttt_list[g_ifttt_number].condition_mask <<= 1;
				g_ifttt_list[g_ifttt_number].condition_mask |= 1;
			}
			g_ifttt_list[g_ifttt_number].condition_mask ^= g_condition_mask;

			g_ifttt_list[g_ifttt_number].action_mask = g_action_mask;
			for(l=0;l<action_number;l++)
			{
				g_ifttt_list[g_ifttt_number].action_mask <<= 1;
				g_ifttt_list[g_ifttt_number].action_mask |= 1;
			}
			g_ifttt_list[g_ifttt_number].action_mask ^= g_action_mask;

			g_ifttt_number ++;

			for(l=0;l<condition_number;l++)
			{
				g_condition_mask <<= 1;
				g_condition_mask |= 1;
			}

			for(l=0;l<action_number;l++)
			{
				g_action_mask <<= 1;
				g_action_mask |= 1;
			}
			vm_log_info("condition_number is %d, action_number %d", condition_number, action_number);
			vm_log_info("g_condition_mask is 0x%x, g_action_mask is 0x%x", g_condition_mask,g_action_mask);
		}
	}

	int num;
	for(num=0;num<g_ifttt_number;num++)
	{
		g_ifttt_list[num].last = 0;
	}

	for(num=0;num<g_ifttt_number;num++)
	{
		unsigned long ifttt_mask;
		unsigned char bit;
		char str[128] = {0};

		ifttt_mask = g_ifttt_list[num].condition_mask;
		for(bit=0;bit<CONDITION_MAX_NUMBER;bit++)
		{
			if((ifttt_mask >> bit) & 0x01)
			{
				strcat(str, if_name[g_condition_list[bit].id]);
				strcat(str,"&");
			}
		}

		str[strlen(str) -1 ] = 0; // Delete the last char "&"
		strcat(str," >> "); // Add " >> "

		ifttt_mask = g_ifttt_list[num].action_mask;
		for(bit=0;bit<ACTION_MAX_NUMBER;bit++)
		{
			if((ifttt_mask >> bit) & 0x01)
			{
				int index;
				for(index=0;index<THEN_NAME_MAX;index++)
				{
					if(g_action_pfunc_list[bit] == g_actuator_do_action_function_list[index]){break;}
				}
				strcat(str, then_name[index]);
				strcat(str,"&");
			}
		}

		str[strlen(str) -1 ] = 0; // Delete the last char "&"

		strncpy(g_ifttt_list[num].name, str, sizeof(g_ifttt_list[num].name));
		g_ifttt_list[num].name[sizeof(g_ifttt_list[num].name)-1]='\0';
	}

	g_ifttt_list[0].prev = 0;
	for(num=0;num<g_ifttt_number-1;num++)
	{
		g_ifttt_list[g_ifttt_number - 1 - num].prev = &g_ifttt_list[g_ifttt_number - 2 - num];
	}

	g_ifttt_last = &g_ifttt_list[g_ifttt_number-1];

	vm_free(p);
}

void ifttt_book_save()
{
	file_delete("ifttt_book.txt");
	file_open("ifttt_book.txt");

	if(g_ifttt_number == 0)return;

	cJSON* pRoot = cJSON_CreateArray();
	if(NULL == pRoot)
	{
		vm_log_info("create array fail");
		cJSON_Delete(pRoot);
		return ;
	}

	int i;
	for(i=0;i<g_ifttt_number;i++)
	{
		unsigned long ifttt_mask;
		unsigned char bit;

		cJSON* pConArr = cJSON_CreateArray();
		if(NULL != pConArr)
		{
			ifttt_mask = ifttt_get(i)->condition_mask;
			for(bit=0;bit<CONDITION_MAX_NUMBER;bit++)
			{
				if((ifttt_mask >> bit) & 0x01)
				{
					cJSON* pConObj = cJSON_CreateObject();
					if(NULL != pConObj)
					{
						cJSON_AddStringToObject(pConObj, "if", if_name[g_condition_list[bit].id]);

						if(g_condition_list[bit].op == '>')cJSON_AddStringToObject(pConObj, "operator", ">");
						else if(g_condition_list[bit].op == '=')cJSON_AddStringToObject(pConObj, "operator", "=");
						else if(g_condition_list[bit].op == '<')cJSON_AddStringToObject(pConObj, "operator", "<");

						switch(g_condition_list[bit].id)
						{
							case 0: // acc.x
							case 1: // acc.y
							case 2: // acc.z
							case 4: // temperature
								cJSON_AddNumberToObject(pConObj, "value", g_condition_list[bit].i32);
							break;
							case 3: // light
							case 9: // button
								cJSON_AddNumberToObject(pConObj, "value", g_condition_list[bit].u32);
							break;
							case 7:// call
								cJSON_AddStringToObject(pConObj, "number", g_condition_list[bit].p);
							break;
							case 8: // sms
								cJSON_AddStringToObject(pConObj, "number", g_condition_list[bit].p);
							break;
							default:
							break;
						}

						cJSON_AddItemToArray(pConArr, pConObj);
					}
					else
					{
						vm_log_info("Create condition object fail");
						cJSON_Delete(pConObj);
						return ;
					}
				}
			}
		}
		else
		{
			vm_log_info("Create condition array fail");
			cJSON_Delete(pConArr);
			return ;
		}

		cJSON* pActArr = cJSON_CreateArray();
		if(NULL != pActArr)
		{
			ifttt_mask = ifttt_get(i)->action_mask;
			for(bit=0;bit<ACTION_MAX_NUMBER;bit++)
			{
				if((ifttt_mask >> bit) & 0x01)
				{
					cJSON* pActObj = cJSON_CreateObject();
					if(NULL != pActObj)
					{
						int index;
						for(index=0;index<THEN_NAME_MAX;index++)
						{
							if(g_action_pfunc_list[bit] == g_actuator_do_action_function_list[index]){break;}
						}
						cJSON_AddStringToObject(pActObj, "then", then_name[index]);

						char temData[2] = {0,'\0'};
						char *temStr;
						switch(index)
						{
							case 0: // led_matrix
								temData[0] = (char)g_action_data_table[bit][0];
								cJSON_AddStringToObject(pActObj, "display", temData);
								cJSON_AddNumberToObject(pActObj, "time", g_action_data_table[bit][1]);
							break;

							case 1: // rgb_ws2812
								switch(g_action_data_table[bit][0] >> 16)
								{
									case 0:
										cJSON_AddStringToObject(pActObj, "mode", "monochrome");
										cJSON_AddNumberToObject(pActObj, "led count", g_action_data_table[bit][0] & 0xffff);
										cJSON_AddNumberToObject(pActObj, "time", g_action_data_table[bit][1]);
										cJSON_AddNumberToObject(pActObj, "color r", (g_action_data_table[bit][2] >> 16) & 0xff );
										cJSON_AddNumberToObject(pActObj, "color g", (g_action_data_table[bit][2] >> 8) & 0xff );
										cJSON_AddNumberToObject(pActObj, "color b", g_action_data_table[bit][2] & 0xff );
									break;

									case 1:
										cJSON_AddStringToObject(pActObj, "mode", "marquee");
										cJSON_AddNumberToObject(pActObj, "led count", g_action_data_table[bit][0] & 0xffff);
										cJSON_AddNumberToObject(pActObj, "time", g_action_data_table[bit][1]);
										cJSON_AddNumberToObject(pActObj, "color r", (g_action_data_table[bit][2] >> 16) & 0xff );
										cJSON_AddNumberToObject(pActObj, "color g", (g_action_data_table[bit][2] >> 8) & 0xff );
										cJSON_AddNumberToObject(pActObj, "color b", g_action_data_table[bit][2] & 0xff );
									break;

									case 2:
										cJSON_AddStringToObject(pActObj, "mode", "rainbow");
										cJSON_AddNumberToObject(pActObj, "led count", g_action_data_table[bit][0] & 0xffff);
										cJSON_AddNumberToObject(pActObj, "time", g_action_data_table[bit][1]);
									break;

									default:
									break;
								}
							break;

							case 2: // music
								cJSON_AddNumberToObject(pActObj, "number", g_action_data_table[bit][0]);
							break;

							case 3: // call
								cJSON_AddStringToObject(pActObj, "number", g_action_data_table[bit][0]);
							break;

							case 4: // sms
								temStr = g_action_data_table[bit][0];
								cJSON_AddStringToObject(pActObj, "number", temStr);
								cJSON_AddStringToObject(pActObj, "content", temStr + 16);
							break;

							default:
							break;
						}

						cJSON_AddItemToArray(pActArr, pActObj);
					}
					else
					{
						vm_log_info("Create action object fail");
						cJSON_Delete(pActObj);
						return ;
					}
				}
			}
		}
		else
		{
			vm_log_info("Create action array fail");
			cJSON_Delete(pActArr);
			return ;
		}

		cJSON* pIftttObj = cJSON_CreateObject();
		if(NULL == pIftttObj)
		{
			vm_log_info("create array fail");
			cJSON_Delete(pIftttObj);
			return ;
		}
		cJSON_AddItemToObject(pIftttObj, "condition", pConArr);
		cJSON_AddItemToObject(pIftttObj, "action", pActArr);

		cJSON_AddItemToArray(pRoot, pIftttObj);
	}

	char* p = cJSON_Print(pRoot, 0);
	file_write("ifttt_book.txt", p, 0);
	vm_free(p);
	cJSON_Delete(pRoot);
}

void ifttt_book_show()
{
	int i;
	unsigned int id;
	long value;

	vm_log_info("g_ifttt_number number is %d", g_ifttt_number);
	for(i=0;i<g_ifttt_number;i++)
	{
		vm_log_info("ifttt%d condition mask is 0x%x, action mask is 0x%x", i, ifttt_get(i)->condition_mask, ifttt_get(i)->action_mask);
		vm_log_info("ifttt%d name is %s", i, ifttt_get(i)->name);
		int j;
		for(j=0;j<CONDITION_MAX_NUMBER;j++)
		{
			if(((ifttt_get(i)->condition_mask) >> j) & 0x01)
			vm_log_info("if type:%d, id:%d, op:%c, Value:%d",g_condition_list[j].type, g_condition_list[j].id, g_condition_list[j].op, g_condition_list[j].i32);
		}
		for(j=0;j<ACTION_MAX_NUMBER;j++)
		{
			if(((ifttt_get(i)->action_mask) >> j) & 0x01)
			vm_log_info("then value:%d,%d,%d",g_action_data_table[j][0], g_action_data_table[j][1], g_action_data_table[j][2], g_action_data_table[j][3]);
		}
	}
}

extern uint8_t g_ifttt_changed;
extern uint8_t g_ifttt_first_visible;
extern uint8_t g_ifttt_current_item;
extern int8_t g_condition_index_map[SENSOR_MAX_NUMBER];
extern uint8_t g_current_condition_position;
extern int8_t g_action_index_map[ACTUATOR_MAX_NUMBER];
extern uint8_t g_current_action_position;

extern void ifttt_list_window_update();

static const char if_short_name[IF_SHORT_NAME_MAX][16] = {"acc_x", "acc_y", "acc_z", "light", "temperature","call", "sms", "button"};
static const char then_short_name[THEN_SHORT_NAME_MAX][16] = {"display", "monochrome", "marquee", "rainbow", "music", "call", "send"};

void ifttt_book_add(char *content)
{
	unsigned char i,j;
	char *str;
	char *str2;
	char *str_if;
	char *str_then;
	char buffer[10] = {'\0'};
	char new_message_content_if[75] = {0,};
	char new_message_content_then[75] = {0,};
	char command_flag = 0;

	condition_t condition_data;
	action_pfunc_t action_funtion;
	unsigned long action_data[3] = {0,};


	str_if = strstr(content, "#IF");
	if(str_if)
	{
		str_then = strstr(content, "THEN");
		if(str_then)
		{

			for(i=0;i < (str_then - str_if);i++)
			{
				new_message_content_if[i] = content[i];
			}
			new_message_content_if[i] = '\0';
			//vm_log_info("new_message_content_if is %s", new_message_content_if);

			for(i=0;content[i] != '\0';i++)
			{
				new_message_content_then[i] = content[str_then - str_if + i];
			}
			//vm_log_info("new_message_content_then is %s", new_message_content_then);

			for(i=0;i<IF_SHORT_NAME_MAX;i++)
			{
				if(strstr(new_message_content_if, if_short_name[i]))break;
			}

			if(i < IF_SHORT_NAME_MAX)
			{
				switch(i)
				{
					case 3: // light
						condition_data.id = i;
						condition_data.type = 0;

						str = strstr(new_message_content_if, "light");
						if(*(str + 6) != '>' && *(str + 6) != '<' && *(str + 6) != '=')return;
						condition_data.op = *(str + 6);

						str2 = strstr(new_message_content_if, ",");
						if((str2 - str) > 8)
						{
							for(j=0;j<(str2 - str - 8);j++)
							{
								buffer[j] = *(str + 8 + j);
							}
							condition_data.u32 = atoi(buffer);
						}
						else return;
					break;

					case 0: // acc_x
					case 1: // acc_y
					case 2: // acc_z
						condition_data.id = i;
						condition_data.type = 1;

						str = strstr(new_message_content_if, "acc_");
						condition_data.op = *(str + 6);
						if(*(str + 6) != '>' && *(str + 6) != '<' && *(str + 6) != '=')return;

						condition_data.i32 = 0;

						str2 = strstr(new_message_content_if, ",");
						if((str2 - str) > 8)
						{
							for(j=0;j<(str2 - str - 8);j++)
							{
								buffer[j] = *(str + 8 + j);
							}
							condition_data.i32 = atoi(buffer);
						}
						else return;
					break;

					case 4: // temperature
						condition_data.id = i;
						condition_data.type = 1;
						str = strstr(new_message_content_if, "temperature");
						condition_data.op = *(str + 12);
						if(*(str + 12) != '>' && *(str + 12) != '<' && *(str + 12) != '=')return;

						condition_data.i32 = 0;
						str2 = strstr(new_message_content_if, ",");
						if((str2 - str) > 14)
						{
							for(j=0;j<(str2 - str - 14);j++)
							{
								buffer[j] = *(str + 14 + j);
							}
							condition_data.i32 = atoi(buffer);
						}
						else return;
					break;

					case 5: // call
					case 6: // sms
						condition_data.id = i + 2;
						condition_data.type = 16;
						condition_data.op = '=';
						condition_data.p = (unsigned long)vm_malloc(16);
						strcpy(condition_data.p, "anyone");
					break;

					case 7: // button
						condition_data.id = i + 2;
						condition_data.type = 0;
						condition_data.op = '=';
						condition_data.u32 = 1;
					break;

					default:
					break;
				}
			}
			else return;

			for(i=0;i<THEN_SHORT_NAME_MAX;i++)
			{
				if(strstr(new_message_content_then, then_short_name[i]))break;
			}
			if(i < THEN_SHORT_NAME_MAX)
			{
				switch(i)
				{
					case 0: // led matrix
						action_funtion = led_matrix_do_action;
						str = strstr(new_message_content_then, "display");
						action_data[0] = *(str+8);
						action_data[1] = 3000;
					break;

					case 1:case 2:case 3: // coloe pixels
						action_funtion = rgb_ws2812_do_action;
						action_data[0] = (i << 16) | 5;
						action_data[1] = 5000;
						action_data[2] = 0xff00ff;
					break;

					case 4: // music
						action_funtion = music_do_action;
						action_data[0] = 0;
					break;

					case 5: // call, need to vm_malloc
						action_funtion = call_do_action;
						action_data[0] = (unsigned long)vm_malloc(16);
						str = strstr(new_message_content_then, "call");
						strcpy(action_data[0], str + 5);
					break;

					case 6: // sms, need to vm_malloc
						action_funtion = sms_do_action;
						action_data[0] = (unsigned long)vm_malloc(56);
						str = strstr(new_message_content_then, "send");
						strcpy(action_data[0], str + 5);
						strcpy(action_data[0] + 16, "Hello, I am RePhone!");
					break;

					default:
					break;
				}
				command_flag  = 1;
			}
			else return;
		}
		else return;
	}
	else	return;
	/*
	if(command_flag)
	{
		switch(condition_data.id)
		{
			case 0: // acc_x
			case 1: // acc_y
			case 2: // acc_z
			case 3: // light
			case 4: // temperature
			case 9: // button
				vm_log_info(" id is %d, type is %d, op is %c, value is %d", condition_data.id, condition_data.type, condition_data.op, condition_data.u32);
			break;

			case 7: // call
			case 8: // sms
				vm_log_info("id is %d, type is %d, op is %c, value is %s", condition_data.id, condition_data.type, condition_data.op, condition_data.u32);
			break;

			default:
			break;
		}
		if(action_funtion == led_matrix_do_action)
		{
			vm_log_info("led_matrix_do_action %c", action_data[0]);
		}
		else if(action_funtion == rgb_ws2812_do_action)
		{
			vm_log_info("rgb_ws2812_do_action 0x%x, 0x%x, 0x%x", action_data[0], action_data[1], action_data[2]);
		}
		else if(action_funtion == music_do_action)
		{
			vm_log_info("music_do_action %d", action_data[0]);
		}
		else if(action_funtion == call_do_action)
		{
			vm_log_info("call_do_action %s", action_data[0]);
		}
		else if(action_funtion == sms_do_action)
		{
			vm_log_info("sms_do_action %s, %s", action_data[0], action_data[0] + 16);
		}
	}
	else
	{
		vm_log_info("Command is error!");
	}
	*/
	if(command_flag == 0)return;

	cJSON* pConArr = cJSON_CreateArray();
	if(pConArr)
	{
		cJSON* pConObj = cJSON_CreateObject();
		if(pConObj)
		{
			cJSON* pConObj = cJSON_CreateObject();
			if(NULL != pConObj)
			{
				cJSON_AddStringToObject(pConObj, "if", if_name[condition_data.id]);

				if(condition_data.op == '>')cJSON_AddStringToObject(pConObj, "operator", ">");
				else if(condition_data.op == '=')cJSON_AddStringToObject(pConObj, "operator", "=");
				else if(condition_data.op == '<')cJSON_AddStringToObject(pConObj, "operator", "<");

				switch(condition_data.id)
				{
					case 0: // acc.x
					case 1: // acc.y
					case 2: // acc.z
					case 4: // temperature
						cJSON_AddNumberToObject(pConObj, "value", condition_data.i32);
					break;
					case 3: // light
					case 9: // button
						cJSON_AddNumberToObject(pConObj, "value", condition_data.u32);
					break;
					case 7:// call
						cJSON_AddStringToObject(pConObj, "number", condition_data.p);
					break;
					case 8: // sms
						cJSON_AddStringToObject(pConObj, "number", condition_data.p);
					break;
					default:
					break;
				}

				cJSON_AddItemToArray(pConArr, pConObj);
			}
			else
			{
				vm_log_info("Create condition object fail");
				cJSON_Delete(pConObj);
				return ;
			}
		}
		else return;
	}
	else return;

	cJSON* pActArr = cJSON_CreateArray();
	if(pActArr)
	{
		cJSON* pActObj = cJSON_CreateObject();
		if(NULL != pActObj)
		{
			int index;
			for(index=0;index<THEN_NAME_MAX;index++)
			{
				if(action_funtion == g_actuator_do_action_function_list[index]){break;}
			}
			cJSON_AddStringToObject(pActObj, "then", then_name[index]);

			char temData[2] = {0,'\0'};
			char *temStr;
			switch(index)
			{
				case 0: // led_matrix
					temData[0] = (char)action_data[0];
					cJSON_AddStringToObject(pActObj, "display", temData);
					cJSON_AddNumberToObject(pActObj, "time", action_data[1]);
				break;

				case 1: // rgb_ws2812
					switch(action_data[0] >> 16)
					{
						case 1:
							cJSON_AddStringToObject(pActObj, "mode", "monochrome");
							cJSON_AddNumberToObject(pActObj, "led count", action_data[0] & 0xffff);
							cJSON_AddNumberToObject(pActObj, "time", action_data[1]);
							cJSON_AddNumberToObject(pActObj, "color r", (action_data[2] >> 16) & 0xff );
							cJSON_AddNumberToObject(pActObj, "color g", (action_data[2] >> 8) & 0xff );
							cJSON_AddNumberToObject(pActObj, "color b", action_data[2] & 0xff );
						break;

						case 2:
							cJSON_AddStringToObject(pActObj, "mode", "marquee");
							cJSON_AddNumberToObject(pActObj, "led count", action_data[0] & 0xffff);
							cJSON_AddNumberToObject(pActObj, "time", action_data[1]);
							cJSON_AddNumberToObject(pActObj, "color r", (action_data[2] >> 16) & 0xff );
							cJSON_AddNumberToObject(pActObj, "color g", (action_data[2] >> 8) & 0xff );
							cJSON_AddNumberToObject(pActObj, "color b", action_data[2] & 0xff );
						break;

						case 3:
							cJSON_AddStringToObject(pActObj, "mode", "rainbow");
							cJSON_AddNumberToObject(pActObj, "led count", action_data[0] & 0xffff);
							cJSON_AddNumberToObject(pActObj, "time", action_data[1]);
						break;

						default:
						break;
					}
				break;

				case 2: // music
					cJSON_AddNumberToObject(pActObj, "number", action_data[0]);
				break;

				case 3: // call
					cJSON_AddStringToObject(pActObj, "number", action_data[0]);
				break;

				case 4: // sms
					temStr = action_data[0];
					cJSON_AddStringToObject(pActObj, "number", temStr);
					cJSON_AddStringToObject(pActObj, "content", temStr + 16);
				break;

				default:
				break;
			}

			cJSON_AddItemToArray(pActArr, pActObj);
		}
		else
		{
			vm_log_info("Create action object fail");
			cJSON_Delete(pActObj);
			return ;
		}
	}
	else return;

	cJSON* pIftttObj = cJSON_CreateObject();
	if(NULL == pIftttObj)
	{
		vm_log_info("create array fail");
		cJSON_Delete(pIftttObj);
		return ;
	}
	cJSON_AddItemToObject(pIftttObj, "condition", pConArr);
	cJSON_AddItemToObject(pIftttObj, "action", pActArr);

	char* buf = cJSON_Print(pIftttObj, 1);

	if(g_ifttt_number == 0)
	{
		file_write("ifttt_book.txt", "[\r]", 0);
		file_write("ifttt_book.txt", "\r\t", -2);
	}
	else file_write("ifttt_book.txt", ",\r\t", -2);

	file_write("ifttt_book.txt", buf, 0);
	file_write("ifttt_book.txt", "\r]", 0);
	vm_free(buf);
	cJSON_Delete(pIftttObj);

	ifttt_book_open();
	ifttt_list_window_update();
	//audioPlay(storageFlash, "bubble.mp3");
}

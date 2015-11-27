
#include "condition.h"
#include <stdlib.h>
#include "vmlog.h"
#include "vmmemory.h"

condition_t g_condition_list[CONDITION_MAX_NUMBER] = {0,};
uint32_t g_condition_mask = 0;


int condition_add(sensor_t *sensor)
{
    int index = 0;
    while (g_condition_mask & (1 << index)) {
        index++;
    }
    if (index >= CONDITION_MAX_NUMBER) {
        return -1;
    }
    
    g_condition_mask |= (uint32_t)1 << index;
    g_condition_list[index].type = sensor->type;
    g_condition_list[index].id = sensor->id;
    g_condition_list[index].u32 = sensor->u32;
    g_condition_list[index].op = '=';
    
    
    if (g_condition_list[index].type > 4) {
        g_condition_list[index].p = vm_malloc(sensor->type);
    }
    
    return index;
}

int condition_check(int index)
{
    condition_t *pcondition;
    sensor_t *psensor;
    
    pcondition = g_condition_list + index;
    psensor = sensor_find(pcondition->id);
    
    if (!psensor) {
        return 0;
    }
    
    if (pcondition->op == '=') {
        if (pcondition->type < P_TYPE) {
            return pcondition->u32 == psensor->u32 ? 1 : 0;
        } else {
            if (pcondition->id == CALL_ID || pcondition->id == SMS_ID) {
                if (*(char *)pcondition->p == 'a' && psensor->p) {
                    return 1;
                }
            }
            
            if (strcmp((char *)pcondition->p, (char *)psensor->p)) {
                return 0;
            } else {
                return 1;
            }
        }
    } else if (pcondition->op == '<') {
        if (pcondition->type == U32_TYPE) {
            return pcondition->u32 > psensor->u32 ? 1 : 0;
        } else if (pcondition->type == I32_TYPE) {
            return pcondition->i32 > psensor->i32 ? 1 : 0;
        } else if (pcondition->type == F32_TYPE) {
            return pcondition->f32 > psensor->f32 ? 1 : 0;
        } else {
            return 0;
        }
    } else if (pcondition->op == '>') {
        if (pcondition->type == U32_TYPE) {
            return pcondition->u32 < psensor->u32 ? 1 : 0;
        } else if (pcondition->type == I32_TYPE) {
            return pcondition->i32 < psensor->i32 ? 1 : 0;
        } else if (pcondition->type == F32_TYPE) {
            return pcondition->f32 < psensor->f32 ? 1 : 0;
        } else {
            return 0;
        }
    }
    
    return 0;
}

condition_t *condition_get(int index)
{
    return g_condition_list + index;
}

void condition_remove(int index)
{
    if (index < CONDITION_MAX_NUMBER && (g_condition_mask >> index) & 1) {
        g_condition_mask &= ~((uint32_t)1 << index);
        
        if (g_condition_list[index].type > 4) {
            vm_malloc(g_condition_list[index].p);
        }
    }
}

void condition_set_value(int index, char *str)
{
    condition_t *ptr = g_condition_list + index;
    
    if (ptr->type == U32_TYPE || ptr->type == I32_TYPE) {
        ptr->i32 = atoi(str);
    } else if (ptr->type == F32_TYPE) {
        ptr->f32 = atof(str);
    } else {
        strcpy((char *)ptr->p, str); 
    }
}

void *condition_get_value(int index)
{
    return g_condition_list[index].p;
}

void condition_set_operator(int index, uint8_t op)
{
    condition_t *ptr = g_condition_list + index;
    ptr->op = op;
}

char *condition_to_string(int index, char *pbuf, int len)
{
    condition_t *pdata = g_condition_list + index;
    int id = pdata->id;
    if (id == BUTTON_ID) {
        snprintf(pbuf, len, "button is pressed");
    } else if (id == CALL_ID || id == SMS_ID) {
        snprintf(pbuf, len, "%s from %s", sensor_info_table[id].name, (char *)(pdata->p));
    } else {
        if (pdata->type == F32_TYPE) {
            snprintf(pbuf, len, "%s %c %0.3f %s", sensor_info_table[id].name, pdata->op, pdata->f32, sensor_info_table[id].unit);
        } else if (pdata->type == I32_TYPE) {
            snprintf(pbuf, len, "%s %c %d %s", sensor_info_table[id].name, pdata->op, pdata->i32, sensor_info_table[id].unit);
        } else {
            snprintf(pbuf, len, "%s %c %u %s", sensor_info_table[id].name, pdata->op, pdata->u32, sensor_info_table[id].unit);
        }
    }

    return pbuf;
}



#include "ifttt.h"
#include "sensor.h"
#include "actuator.h"
#include <string.h>

ifttt_t g_ifttt_list[IFTTT_MAX_NUMBER] = {0,};
ifttt_t *g_ifttt_last = g_ifttt_list;
int g_ifttt_number = 0;
char g_ifttt_description_string[64];

void ifttt_check()
{
    int i;
    int condition_mask;
    int condition_index;
    int action_mask;
    int action_index;
    int result;
    ifttt_t *ptr = g_ifttt_last;
    for (i = 0; i < g_ifttt_number; i++) {
        result = 0;
        condition_index = 0;
        condition_mask = ptr->condition_mask;
        while (condition_mask) {
            if (condition_mask & 1) {
                if (condition_check(condition_index)) {
                    result++;
                } else {
                    result -= 32;      // make sure that result < 0
                }
            }
            condition_mask >>= 1;
            condition_index++;
        }
        
        if (result > 0) {
            if (!ptr->last) {
                action_index = 0;
                action_mask = ptr->action_mask;
                while (action_mask) {
                    if (action_mask & 1) {
                        action_exec(action_index);
                    }
                    action_mask >>= 1;
                    action_index++;
                }
                
                ptr->last = 1;
            }
        } else {
            ptr->last = 0;
        }
        
        ptr = ptr->prev;
    }
}

int ifttt_add(int8_t *condition_index_map, int8_t *action_index_map)
{
    int i;
    uint32_t conditions = 0;
    uint32_t actions = 0;
    for (i = 0; i < IFTTT_MAX_NUMBER; i++) {
        if (g_ifttt_list[i].condition_mask == 0) {
            break;
        }
    }
    
    if (i >= IFTTT_MAX_NUMBER) {
        return -1;
    }
    
    g_ifttt_list[i].prev = g_ifttt_last;
    g_ifttt_last = &g_ifttt_list[i];
    
    g_ifttt_last->name[0] = 0;
    for (i = 0; i < SENSOR_MAX_NUMBER; i++) {
        int condition_index = condition_index_map[i];
        if (condition_index != -1) {
            if (!conditions){
                strcat(g_ifttt_last->name, sensor_get_name(i));
            } else {
                strcat(g_ifttt_last->name, "&");
                strcat(g_ifttt_last->name, sensor_get_name(i));
            }
            conditions |= (uint32_t)1 << condition_index_map[i];
        }
    }
    
    if (conditions == 0) {
        return -1;
    }
    
    strcat(g_ifttt_last->name, " >> ");
    for (i = 0; i < ACTUATOR_MAX_NUMBER; i++) {
        int action_index = action_index_map[i];
        if (action_index != -1) {
            if (!actions) {
                strcat(g_ifttt_last->name, actuator_get_name(i));
            } else {
                strcat(g_ifttt_last->name, "&");
                strcat(g_ifttt_last->name, actuator_get_name(i));
            }
            actions |= (uint32_t)1 << action_index_map[i];
        }
    }
    
    if (actions == 0) {
        return -1;
    }
    
    g_ifttt_last->condition_mask = conditions;
    g_ifttt_last->action_mask = actions;
    g_ifttt_last->last = 0;
    g_ifttt_number++;
    
    return 0;
}

void ifttt_remove(int index)
{
    ifttt_t **pptr;
    ifttt_t *p_remove_item;
    int n;
    int i;
    
    if (index >= g_ifttt_number) {
        return;
    }
    
    g_ifttt_number--;
    
    n = g_ifttt_number - index;
    pptr = &g_ifttt_last;
    p_remove_item = g_ifttt_last;
    for (i = 0; i < n; i++) {
        pptr = &(p_remove_item->prev);
        p_remove_item = p_remove_item->prev;
    }
    *pptr = p_remove_item->prev;
    p_remove_item->prev = 0;
    p_remove_item->condition_mask = 0;
}

ifttt_t *ifttt_get(int index)
{
    int n = g_ifttt_number - index - 1;
    ifttt_t *ptr;
    int i;

    ptr = g_ifttt_last;
    for (i = 0; i < n; i++) {
        ptr = ptr->prev;
    }

    return ptr;
}

int ifttt_get_number()
{
    return g_ifttt_number;
}

char *ifttt_get_name(int index)
{
    int n = g_ifttt_number - index - 1;
    int i;
    ifttt_t *ptr = g_ifttt_last;

    for (i = 0; i < n; i++) {
        ptr = ptr->prev;
    }
    return ptr->name;
}

char *ifttt_get_description(int index)
{
    uint32_t condition_mask;
    uint32_t action_mask;
    int i;
    int conditions = 0;
    int actions = 0;
    ifttt_t *ptr = ifttt_get(index);
    
    condition_mask = ptr->condition_mask;
    g_ifttt_description_string[0] = '\0';
    i = 0;
    strcat(g_ifttt_description_string, "if  ");
    while (condition_mask) {
        if (condition_mask & 1) {
            int len = strlen(g_ifttt_description_string);
            if (conditions > 0) {
                strcat(g_ifttt_description_string, " & ");
                len += 3;
            }
            condition_to_string(i, g_ifttt_description_string + len, sizeof(g_ifttt_description_string) - len);
            conditions++;
        }

        condition_mask >>= 1;
        i++;
    }
    
    action_mask = ptr->action_mask;
    i = 0;
    strcat(g_ifttt_description_string, "  then  ");
    while (action_mask) {
        if (action_mask & 1) {
            int len = strlen(g_ifttt_description_string);
            if (actions > 0) {
                strcat(g_ifttt_description_string, " & ");
                len += 3;
            }
            action_to_string(i, g_ifttt_description_string + len, sizeof(g_ifttt_description_string) - len);
            actions++;
        }

        action_mask >>= 1;
        i++;
    }

    return g_ifttt_description_string;
}


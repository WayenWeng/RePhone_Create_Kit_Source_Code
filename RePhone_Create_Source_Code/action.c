
#include "action.h"
#include "actuator.h"
#include "vmmemory.h"

action_pfunc_t g_action_pfunc_list[ACTION_MAX_NUMBER] = {0,};
uint32_t g_action_data_table[ACTION_MAX_NUMBER][3] = {0,};
uint32_t g_action_mask = 0;

extern void call_do_action(uint32_t *pdata);
extern void sms_do_action(uint32_t *pdata);

int action_add(action_pfunc_t pfunc)
{
    int index = 0;
    while (g_action_mask & (1 << index)) {
        index++;
    }
    if (index >= ACTION_MAX_NUMBER) {
        return -1;
    }
    
    if (pfunc == call_do_action) {
        g_action_data_table[index][0] = (uint32_t)vm_malloc(16);
    } else if (pfunc == sms_do_action) {
        g_action_data_table[index][0] = (uint32_t)vm_malloc(16 + 40);
    }
    
    g_action_mask |= (uint32_t)1 << index;
    g_action_pfunc_list[index] = pfunc;
    
    return index;
}

int action_exec(int action_id)
{
    action_pfunc_t pfunc;
    if ((action_id >= ACTION_MAX_NUMBER) || !((g_action_mask >> action_id) & 1)) {
        return 0;
    }
    
    pfunc = g_action_pfunc_list[action_id];
    if (pfunc) {
        return pfunc(g_action_data_table[action_id]);
    }
        
    return 0;
}

void action_remove(int action_id)
{
    if ((action_id < ACTION_MAX_NUMBER) && ((g_action_mask >> action_id) & 1)) {
        if (g_action_pfunc_list[action_id] == call_do_action) {
            vm_free((void *)g_action_data_table[action_id][0]);
        } else if (g_action_pfunc_list[action_id] == sms_do_action) {
            vm_free((void *)g_action_data_table[action_id][0]);
        }
        
        g_action_mask &= ~((uint32_t)1 << action_id);
        g_action_pfunc_list[action_id] = 0;
    }
}

void action_set_function(int index, int (*pfunc)(void *))
{
    g_action_pfunc_list[index] = pfunc;
}

uint32_t *action_get_data(int index)
{
    return g_action_data_table[index];
}

char *action_to_string(int index, char *pbuf, int len)
{
    if (g_action_pfunc_list[index] == call_do_action) {
        snprintf(pbuf, len, "call %s", (char *)g_action_data_table[index][0]);
    }  else if (g_action_pfunc_list[index] == sms_do_action) {
        snprintf(pbuf, len, "text %s", (char *)g_action_data_table[index][0]);
    } else {
        snprintf(pbuf, len, "set %s", actuator_get_action_name(g_action_pfunc_list[index]));
    }
    return pbuf;
}


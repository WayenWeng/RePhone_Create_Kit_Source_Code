
#ifndef __ACTUATOR_H__
#define __ACTUATOR_H__

#include "action.h"

#define ACTUATOR_MAX_NUMBER 16
#define ACTUATOR_HIDDEN_NUMBER 2

typedef void (*actuator_pfunc_t)(uint32_t *pdata);

int actuator_get_number();
char *actuator_get_name(int index);
action_pfunc_t actuator_get_action_function(int index);
int actuator_get_action_data(int index, uint32_t *pdata);
char *actuator_get_action_name(actuator_pfunc_t pfunc);

#endif // __ACTUATOR_H__

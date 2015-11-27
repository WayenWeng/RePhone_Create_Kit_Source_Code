
#ifndef __ACTION_H__
#define __ACTION_H__

#include <stdint.h>

#define ACTION_MAX_NUMBER   32

typedef int (*action_pfunc_t)(void *);

int action_add(action_pfunc_t pfunc);
void action_remove(int action_id);
int action_exec(int action_id);
void action_set_function(int index, int (*pfunc)(void *));
uint32_t *action_get_data(int index);
char *action_to_string(int index, char *pbuf, int len);

#endif // __ACTION_H__

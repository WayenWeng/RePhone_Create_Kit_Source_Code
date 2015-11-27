
#ifndef __CONDITION_H__
#define __CONDITION_H__

#include "sensor.h"

#include <stdint.h>

#define CONDITION_MAX_NUMBER   32


typedef struct {
    uint8_t   type;
    uint8_t   op;
    uint16_t  id;
    union {
        uint32_t u32;
        int32_t  i32;
        float    f32;
        void    *p;
    };
} condition_t;

extern condition_t g_condition_list[CONDITION_MAX_NUMBER];

int condition_add(sensor_t *sensor);
int condition_check(int index);
condition_t *condition_get(int index);
void condition_remove(int index);
void condition_set_value(int index, char *str);
void *condition_get_value(int index);
void condition_set_operator(int index, uint8_t op);
char *condition_to_string(int index, char *pbuf, int len);

#endif // __CONDITION_H__

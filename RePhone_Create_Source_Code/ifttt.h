
#ifndef __IFTTT_H__
#define __IFTTT_H__

#include <stdint.h>

#define IFTTT_MAX_NUMBER    16

struct _ifttt_t {
    struct _ifttt_t *prev;
    uint32_t condition_mask;
    uint32_t action_mask;
    uint8_t last;
    char name[32];
};

typedef struct _ifttt_t  ifttt_t;

void ifttt_check();
int ifttt_add(int8_t *condition_index_map, int8_t *action_index_map);
void ifttt_remove(int index);
int ifttt_get_number();
char *ifttt_get_name(int index);
char *ifttt_get_description(int index);
ifttt_t *ifttt_get(int index);

#endif // __IFTTT_H__

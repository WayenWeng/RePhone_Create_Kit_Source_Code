

#ifndef __SENSOR_H__
#define __SENSOR_H__

#include <stdint.h>

#define SENSOR_MAX_NUMBER  32
#define SENSOR_HIDDEN_NUMBER 3

#define U32_TYPE    0
#define I32_TYPE    1
#define F32_TYPE    2
#define P_TYPE      4 // or > 4

enum sensor_id_t {
  ACC_X_ID = 0,
  ACC_Y_ID = 1,
  ACC_Z_ID = 2,
  LIGHT_ID = 3,
  TEMPERATURE_ID = 4,
  HUMIDITY_ID = 5,
  ACC_ID   = 6,
  CALL_ID  = 7,
  SMS_ID   = 8,
  BUTTON_ID = 9
};


typedef struct {
    const char   *name;
    const char   *unit;
} sensor_info_t;

typedef struct {
    uint8_t   type;
    uint8_t   reserved;
    uint16_t  id;
    union {
        uint32_t u32;
        int32_t  i32;
        float    f32;
        void    *p;
    };
} sensor_t;

extern const sensor_info_t sensor_info_table[];

void sensor_scan();
sensor_t *sensor_find(uint16_t id);
sensor_t *sensor_get(int index);
int sensor_get_id(int index);
int sensor_get_index(uint16_t id);
char *sensor_get_name(int index);
void sensor_update();
int sensor_get_number();
char *sensor_to_string(int index, char *pbuf, int len);
char *sensor_set_title(int index, char *pbuf, int len);
void sensor_set_update_callback(void (*pfunc)(void));

#endif // __SENSOR_H__

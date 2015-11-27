

#include "ldlci2cV2.h"
#include "sensor.h"
#include "ifttt.h"


const sensor_info_t sensor_info_table[] = {
  {"acc.x", "mG"},
  {"acc.y", "mG"},
  {"acc.z", "mG"},
  {"light", "Lux"},
  {"temperature", "C"},
  {"humidity", "%"},
  {"acc", "mG"},
  {"call", ""},
  {"sms", ""},
  {"button", ""},
};

static int g_sensor_number = 0;
static sensor_t g_sensor_list[SENSOR_MAX_NUMBER] = {0,};
static uint32_t g_new_sensor_data_mask = 0;
static void (*p_sensor_changed_callback)(void) = 0;

void sensor_scan()
{
    sensor_t *psensor;
    uint32_t device_info;
    uint8_t number;
    int i;
    
    dlc_i2c_configure(0x03, 100);
    dlc_i2c_receive(0x0, &device_info, 4);
    
    number = device_info >> 24;
    
    psensor = g_sensor_list + g_sensor_number;
    for (i = 0; i < number; i++) {
        dlc_i2c_receive(4 + 8 * i, psensor, 8);
        psensor++;
        g_new_sensor_data_mask |= (uint32_t)1 << i;
    }
    
    g_sensor_number += number;
    
    psensor->type = 16; // P_TYPE, require 16 extra bytes memory
    psensor->id = CALL_ID;
    psensor->p = 0;
    g_sensor_number++;
    
    psensor++;
    psensor->type = 16; // P_TYPE, require 16 extra bytes memory
    psensor->id = SMS_ID;
    psensor->p = 0;
    g_sensor_number++;
    
    psensor++;
    psensor->type = U32_TYPE;
    psensor->id = BUTTON_ID;
    psensor->u32 = 0;
    g_sensor_number++;
}

int sensor_get_number()
{
    return g_sensor_number;
}

void sensor_update()
{
    uint32_t device_info;
    uint32_t new_sensor_data_flag;
    sensor_t sensor;
    int sensor_index;
    int i;

    dlc_i2c_configure(0x03, 100);

    // flag
    dlc_i2c_receive(0x0, &device_info, 4);
    new_sensor_data_flag = device_info & 0x0FFF;
    
    i = 0;
    while (new_sensor_data_flag) {
        if (new_sensor_data_flag & 1) {
            dlc_i2c_receive(4 + 8 * i, &sensor, 8);
            
            sensor_index = sensor_get_index(sensor.id);
            if (sensor_index >= 0) {
                g_sensor_list[sensor_index].u32 = sensor.u32;
                
                g_new_sensor_data_mask |= (uint32_t)1 << sensor_index;
            } else {
                
            }
        }
        
        i++;
        new_sensor_data_flag >>= 1;
    }

    if (p_sensor_changed_callback) {
        p_sensor_changed_callback();
    }
    
    ifttt_check();
}

sensor_t *sensor_find(uint16_t id)
{
    int i;
    for (i = 0; i < g_sensor_number; i++) {
        if (g_sensor_list[i].id == id) {
            return g_sensor_list + i;
        }
    }
    
    return 0;
}

sensor_t *sensor_get(int index)
{
    return g_sensor_list + index;
}

int sensor_get_id(int index)
{
    return g_sensor_list[index].id;
}

int sensor_get_index(uint16_t id)
{
    int i;
    for (i = 0; i < g_sensor_number; i++) {
        if (g_sensor_list[i].id == id) {
            return i;
        }
    }
    
    return -1;
}

char *sensor_get_name(int index)
{
    return sensor_info_table[g_sensor_list[index].id].name;
}

char *sensor_to_string(int index, char *pbuf, int len)
{
    sensor_t *pdata = g_sensor_list + index;
    int id = pdata->id;
    if (pdata->type == F32_TYPE) {
        snprintf(pbuf, len, "%s: %0.3f %s", sensor_info_table[id].name, pdata->f32, sensor_info_table[id].unit);
    } else if (pdata->type == I32_TYPE) {
        snprintf(pbuf, len, "%s: %d %s", sensor_info_table[id].name, pdata->i32, sensor_info_table[id].unit);
    } else if (pdata->type == U32_TYPE) {
        snprintf(pbuf, len, "%s: %u %s", sensor_info_table[id].name, pdata->u32, sensor_info_table[id].unit);
    } else {
        snprintf(pbuf, len, "%s: %s", sensor_info_table[id].name, (char *)pdata->p);
    }
    
    return pbuf;
}

char *sensor_set_title(int index, char *pbuf, int len)
{
    sensor_t *pdata = g_sensor_list + index;
    int id = pdata->id;
    snprintf(pbuf, len, "Set %s", sensor_info_table[id].name);

    return pbuf;
}

void sensor_set_update_callback(void (*pfunc)(void))
{
    p_sensor_changed_callback = pfunc;
}

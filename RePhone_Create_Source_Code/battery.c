

#include "battery.h"
#include "ldlcgpio.h"

int battery_get_level()
{
//    int level = vm_pwr_get_battery_level();
    int level = analogRead(0);
    if (level < 840) {
        level = 840;
    }
    return (level - 840) * 100 / (1023 - 840);
}

int battery_is_charging()
{
    if (vm_pwr_is_charging())
        return 1;
    else
        return 0;
}


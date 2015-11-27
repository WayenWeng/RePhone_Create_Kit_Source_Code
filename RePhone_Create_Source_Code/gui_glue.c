#include "ugui.h"
#include "vmgraphic.h"
#include "vmtimer.h"
#include "vmlog.h"
#include "vmtouch.h"
#include "vmdcl.h"
#include "string.h"

vm_graphic_frame_t g_frame;
vm_graphic_frame_t g_rotated_frame;
const vm_graphic_frame_t* g_frame_blt_group[1];

UG_GUI g_gui;
uint8_t g_gui_changed = 0;
UG_COLOR g_gui_last_color = 0xFFFFFFFF;

const UG_FONT FONT_SIZE20 = {(unsigned char*)0,FONT_TYPE_1BPP,12,20,0,255,NULL};
const UG_FONT FONT_SIZE40 = {(unsigned char*)0,FONT_TYPE_1BPP,24,40,0,255,NULL};

/* GUI update timer */
VM_TIMER_ID_PRECISE g_timer_id;

extern void time_update_callback(void);
extern int screen_suspend(void);
extern void screen_resume(void);


void gui_draw_font( char chr, UG_S16 x, UG_S16 y, UG_COLOR fc, UG_COLOR bc, const UG_FONT* font)
{
    char str[2] = {0, 0};
    VMWCHAR s[4];                 /* string's buffer */
    VMUINT32 size;
    vm_graphic_color_argb_t color;      /* use to set screen and text color */

    uint16_t height = font->char_height;

    str[0] = chr;
    vm_chset_ascii_to_ucs2(s, 4, str);

    /* set color and draw text*/
    if (g_gui_last_color != fc) {
        vm_graphic_color_argb_t color;

        color.a = (uint8_t)(fc >> 24);
        color.r = (uint8_t)(fc >> 16);
        color.g = (uint8_t)(fc >> 8);
        color.b = (uint8_t) fc;

        vm_graphic_set_color(color);
        g_gui_last_color = fc;
    }
    vm_graphic_set_font_size(height);
    vm_graphic_draw_text_by_baseline(&g_frame, x + 1, y + height, s, 0);

    g_gui_changed = 1;
}

void gui_change_color(UG_COLOR c)
{
    if (g_gui_last_color != c) {
        vm_graphic_color_argb_t color;
        color.a = (uint8_t)(c >> 24);
        color.r = (uint8_t)(c >> 16);
        color.g = (uint8_t)(c >> 8);
        color.b = (uint8_t) c;
        vm_graphic_set_color(color);
        g_gui_last_color = c;
    }
}

static void gui_draw_point(UG_S16 x, UG_S16 y, UG_COLOR c)
{
#if 0
    if (g_gui_last_color != c) {
        vm_graphic_color_argb_t color;
        color.a = (uint8_t)(c >> 24);
        color.r = (uint8_t)(c >> 16);
        color.g = (uint8_t)(c >> 8);
        color.b = (uint8_t) c;
        vm_graphic_set_color(color);
        g_gui_last_color = c;
    }
    vm_graphic_draw_point(&g_frame, x, y);
#else
    uint16_t *pbuf = (uint16_t *)g_frame.buffer; 
    pbuf += y * 240 + x;
    *pbuf = ((c >> 8) & (0x1F << 11)) | ((c >> 5) & (0x3F << 5)) | ((c >> 3) & 0x1F);
#endif

    g_gui_changed = 1;
}

static UG_RESULT gui_draw_line(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2,
        UG_COLOR c)
{
    if (g_gui_last_color != c) {
        vm_graphic_color_argb_t color;
        color.a = (uint8_t)(c >> 24);
        color.r = (uint8_t)(c >> 16);
        color.g = (uint8_t)(c >> 8);
        color.b = (uint8_t) c;
        vm_graphic_set_color(color);
        g_gui_last_color = c;
    }
    vm_graphic_draw_line(&g_frame, x1, y1, x2, y2);

    g_gui_changed = 1;
    return UG_RESULT_OK;
}

static UG_RESULT gui_fill_rectangle(UG_S16 x1, UG_S16 y1, UG_S16 x2, UG_S16 y2,
        UG_COLOR c)
{
    if (g_gui_last_color != c) {
        vm_graphic_color_argb_t color;
        color.a = (uint8_t)(c >> 24);
        color.r = (uint8_t)(c >> 16);
        color.g = (uint8_t)(c >> 8);
        color.b = (uint8_t) c;
        vm_graphic_set_color(color);
        g_gui_last_color = c;
    }
    vm_graphic_draw_solid_rectangle(&g_frame, x1, y1, x2 - x1 + 1, y2 - y1 + 1);

    g_gui_changed = 1;
    return UG_RESULT_OK;
}

static void gui_timer_callback(VM_TIMER_ID_PRECISE tid, void* user_data)
{
    static int count = 0;
    vm_graphic_point_t positions[1] = { 0, 0 };
    
    if (screen_suspend()) {
    	return;
    }

    count++;
    if (count >= 10) {
        count = 0;
        time_update_callback();
    }

    UG_Update();

    if (g_gui_changed) {
        vm_graphic_rotate_frame(&g_rotated_frame, &g_frame, VM_GRAPHIC_ROTATE_180);
        vm_graphic_blt_frame(g_frame_blt_group, positions, 1);
        g_gui_changed = 0;
    }
}

void handle_touchevt(VM_TOUCH_EVENT event, VMINT x, VMINT y)
{
    vm_log_info("touch event=%d,touch x=%d,touch y=%d", event, x, y);
    /* output log to monitor or catcher */
    
    screen_resume();

    y = 239 - y - 8;
    x = 239 - x;

    if (event == VM_TOUCH_EVENT_TAP) {
        UG_TouchUpdate(x, y, TOUCH_STATE_PRESSED);
    } else if (event == VM_TOUCH_EVENT_RELEASE) {
        UG_TouchUpdate(x, y, TOUCH_STATE_RELEASED);
    }

    UG_Update();

    if (g_gui_changed) {
        vm_graphic_point_t positions[1] = { 0, 0 };

        vm_graphic_rotate_frame(&g_rotated_frame, &g_frame, VM_GRAPHIC_ROTATE_180);
        vm_graphic_blt_frame(g_frame_blt_group, positions, 1);
        g_gui_changed = 0;
    }
}

void gui_setup(void)
{
    // Initialize lcd and touch panel
    g_frame.width = 240;
    g_frame.height = 240;
    g_frame.color_format = VM_GRAPHIC_COLOR_FORMAT_16_BIT;
    g_frame.buffer = (VMUINT8*) vm_malloc_dma(
            g_frame.width * g_frame.height * 2);
    g_frame.buffer_length = (g_frame.width * g_frame.height * 2);

    g_rotated_frame.width = 240;
    g_rotated_frame.height = 240;
    g_rotated_frame.color_format = VM_GRAPHIC_COLOR_FORMAT_16_BIT;
    g_rotated_frame.buffer = (VMUINT8*) vm_malloc_dma(
            g_rotated_frame.width * g_rotated_frame.height * 2);
    g_rotated_frame.buffer_length = (g_rotated_frame.width * g_rotated_frame.height * 2);

    g_frame_blt_group[0] = &g_rotated_frame;

    UG_Init(&g_gui, gui_draw_point, 240, 240);
    UG_SelectGUI(&g_gui);
    UG_DriverRegister(DRIVER_DRAW_LINE, (void *) gui_draw_line);
    UG_DriverRegister(DRIVER_FILL_FRAME, (void *) gui_fill_rectangle);
    UG_DriverEnable(DRIVER_DRAW_LINE);
    UG_DriverEnable(DRIVER_FILL_FRAME);

    // vm_touch_register_event_callback(handle_touchevt);

    /* create GUI update timer */
    //g_timer_id = vm_timer_create_precise(100, gui_timer_callback, NULL);
    g_timer_id = vm_timer_create_non_precise(100, gui_timer_callback, NULL);
}


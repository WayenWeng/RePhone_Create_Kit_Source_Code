
#include "vmtype.h"
#include "ugui.h"
#include "laudio.h"
#include "vmfs.h"
#include "vmchset.h"

UG_WINDOW g_music_window;
char g_music_filename[4][32] = {0,};
int g_music_filename_selected = -1;
uint32_t *g_music_data_ptr = NULL;

void music_window_show(uint32_t *pdata)
{
    g_music_data_ptr = pdata;
    UG_WindowShow(&g_music_window);
}

void music_do_action(uint32_t *pdata)
{
    char *music = "ringtone.mp3";
    unsigned index = *pdata;
    if (index < 4 && g_music_filename[index][0] != '\0') {
        music = g_music_filename[index];
    }
    audioPlay(storageFlash, music);
}

void music_window_callback(UG_MESSAGE *msg)
{
    if (msg->type == MSG_TYPE_OBJECT) {
        if (msg->id == OBJ_TYPE_BUTTON && msg->event == OBJ_EVENT_RELEASED) {
            int index;
            switch (msg->sub_id) {
                case 0: // back
                    UG_WindowShow(UG_GetLastWindow());
                    g_music_data_ptr = NULL;
                    break;
                case 1: // stop
                    audioStop(NULL);
                    break;
                case 2: // play
                    //audioPlay(storageFlash, "ringtone.mp3");
                    audioPlay(storageFlash, g_music_filename[g_music_filename_selected]);
                    break;
                case 3:
                case 4:
                case 5:
                case 6:
                    index = msg->sub_id - 3;
                    if (g_music_filename[index][0] != 0) {
                        if (g_music_filename_selected >= 0) {
                            UG_ButtonSetForeColor(&g_music_window, 3 + g_music_filename_selected, C_WHITE);
                            UG_ButtonSetBackColor(&g_music_window, 3 + g_music_filename_selected, C_BUTTON_BC);
                        }

                        UG_ButtonSetForeColor(&g_music_window, 3 + index, C_BUTTON_BC);
                        UG_ButtonSetBackColor(&g_music_window, 3 + index, C_WHITE);

                        g_music_filename_selected = index;
                        if (g_music_data_ptr) {
                            *g_music_data_ptr = index;
                        }
                    }
                    break;
            }

        }
    }
}

void music_window_create(void)
{
    static UG_BUTTON buttons[8];
    static UG_OBJECT objects[8];
    char *icons[] = { "1", "D", "C" }; // back, stop, play
    int i = 0;
    int id = 0;

    UG_WindowCreate(&g_music_window, objects,
            sizeof(objects) / sizeof(*objects), music_window_callback);
    UG_WindowSetStyle(&g_music_window, WND_STYLE_2D);

    for (i = 0; i < 3; i++) {
        UG_ButtonCreate(&g_music_window, buttons+ i, i, 80 * i,
                200, 80 * i + 80 - 1, 239);
        UG_ButtonSetFont(&g_music_window, i , &FONT_ICON24);
        UG_ButtonSetText(&g_music_window, i, icons[i]);
        UG_ButtonSetStyle(&g_music_window, i,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

    }

    {
        vm_fs_info_t fs_info;
        VM_FS_HANDLE handle;
        char search_pattern[32];
        VMWCHAR search_pattern_w[32];

        snprintf(search_pattern, sizeof(search_pattern), "%c:\\*.mp3", vm_fs_get_internal_drive_letter());
        vm_chset_ascii_to_ucs2(search_pattern_w, sizeof(search_pattern_w), search_pattern);

        handle = vm_fs_find_first(search_pattern_w, &fs_info);
        if (handle > 0) {
            for (i = 0; i < 4; i++) {
                vm_chset_ucs2_to_ascii(g_music_filename[i], sizeof(g_music_filename[i]), fs_info.filename);

                if (vm_fs_find_next(handle, &fs_info) != VM_FS_SUCCESS) {
                    break;
                }
            }
            vm_fs_find_close(handle);
        }
    }

    for (i = 0; i < 4; i++) {
        id = 3 + i;
        UG_ButtonCreate(&g_music_window, buttons + id, id, 0,
                40 * i + 40, 239, 40 * i + 80 - 1);
        UG_ButtonSetFont(&g_music_window, id, &FONT_SIZE20);
        UG_ButtonSetText(&g_music_window, id, g_music_filename[i]);
        UG_ButtonSetStyle(&g_music_window, id,
                BTN_STYLE_2D | BTN_STYLE_TOGGLE_COLORS | BTN_STYLE_NO_BORDERS);

    }

    if (g_music_filename[0][0] != 0) {
        g_music_filename_selected = 0;

        UG_ButtonSetForeColor(&g_music_window, 3, C_BUTTON_BC);
        UG_ButtonSetBackColor(&g_music_window, 3, C_WHITE);
    }



    id = 7;
    UG_ButtonCreate(&g_music_window, buttons + id, id, 0, 0, 239, 39);
    UG_ButtonSetFont(&g_music_window, id, &FONT_SIZE20);
    UG_ButtonSetText(&g_music_window, id, "Music");
    UG_ButtonSetStyle(&g_music_window, id,
            BTN_STYLE_2D | BTN_STYLE_NO_BORDERS);
    UG_ButtonSetBackColor(&g_music_window, id, 0x000000);
}

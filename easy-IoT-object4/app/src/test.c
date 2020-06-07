#define DEBUG

#include "drivers.h"
#include "app.h"
#include "api.h"

#include "test.h"
#include "web_cfg.h"
#include <cctype>


u8_t  command[100] = {0,};
int cmd_index = 0;

void handle_cmd(void)
{
    //sprintf(cmd_str, "user cmd:[%02x]", cmd);
    int i = 0;

#if BAIDU_AI_VOICE_ENABLE
    sleep(1000);    //wait some time for all data
    for(i = 0; i < cmd_index; i++)
        p_dbg("user cmd:[%02x]", command[i]);
    send_and_encode_text_event(command, cmd_index);
       
    cmd_index = 0;
    return;
#endif
#if ENABLE_LCD == 1
    gui_show_info("handle usart data");
#endif
}


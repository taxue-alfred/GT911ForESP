#include <stdio.h>

#include "Alfred_GT911.h"

#define TOUCH_GT911_SDA 41
#define TOUCH_GT911_SCL 42
#define SCREEN_WIDTH 480
#define SCREEN_HEIGHT 480

Alfred_GT911 alfredGt911;

void GT911_test(void * pvPara)
{
    int x, y;
    while(1){
        if(GT911_touched(&alfredGt911)){
            //循环读取每个触控点的位置值
            for (int i = 0; i < 5; ++i) {
                GT911_read_pos(&alfredGt911, &x, &y, i);
                printf("No: %d, touched x: %d, touched y: %d\n", i,  x, y);
            }
        }
        vTaskDelay(100 / portTICK_PERIOD_MS);
    }
}

void app_main(void)
{
    GT911_init(&alfredGt911, TOUCH_GT911_SDA, TOUCH_GT911_SCL, -1, -1, I2C_NUM_0,
               GT911_ADDR1, SCREEN_WIDTH, SCREEN_HEIGHT);
    GT911_setRotation(&alfredGt911, ROTATION_NORMAL);

    xTaskCreate(GT911_test, "GT911_test", 1024 * 3,
                NULL, 12, NULL);
}

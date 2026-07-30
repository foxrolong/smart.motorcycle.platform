
// thư viện không thể thiếu khi lập trình esp-idf
// thư viện chuẩn của C/C++
#include <stdio.h>
#include <string.h>
#include "wifi_board.h" // thư viện wifi_board
#include "display/lcd_display.h" // thư viện lcd_display
#include "config.h" // thư viện config

#include "led.h"// thư viện led_strip
//thư viện driver 
// #include "driver/gpio.h"
// #include "driver/uart.h"
//
#include "esp_system.h"// thư viện hệ thống esp-idf

#define led_on_ms 2000

void app_main(void)
{
    while(1){
        rgb_led_on();
        vTaskDelay(led_on_ms / portTICK_PERIOD_MS);
        rgb_led_off();
        vTaskDelay(led_on_ms / portTICK_PERIOD_MS);
    }
}

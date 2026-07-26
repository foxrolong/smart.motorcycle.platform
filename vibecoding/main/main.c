@file main.c


// thư viện không thể thiếu khi lập trình esp-idf
// thư viện chuẩn của C/C++
#include <stdio.h>
#include <string.h>
// thư viện hệ điều hành FreeRTOS
#include "freertos/FreeRTOS.h" 
#include "freertos/task.h"
// thư viện của esp-idf
#include "esp_log.h"
#include "esp_err.h"
//thư viện driver 
#include "driver/gpio.h"
#include "driver/uart.h"
//
#include "esp_system.h"

#define led_on_ms 2000

void app_main(void)
{
    
}

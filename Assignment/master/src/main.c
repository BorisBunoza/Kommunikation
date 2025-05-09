/**
 * @file main.c
 * @author Faroch Mehri (faroch.mehri@ya.se)
 * @brief An example of I2C controller in master mode.
 *        Look at: https://docs.espressif.com/projects/esp-idf/en/stable/esp32c6/api-reference/peripherals/i2c.html
 *
 * @version 0.1
 * @date 2025-04-01
 *
 * @copyright Copyright (c) 2025
 *
 */
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <esp_log.h>
#include <esp_random.h>
#include <driver/gpio.h>
#include <esp_task_wdt.h>
#include <driver/i2c_master.h>
#include <bootloader_random.h>

#define MASTER I2C_NUM_0
#define PIN_SDA GPIO_NUM_6 // GPIO for SDA
#define PIN_SCL GPIO_NUM_7 // GPIO for SCL
#define CLOCK_RATE 400000  // Communication Speed
#define SLAVE_ADDRESS 0x55 // Slave Address
#define MSG_SIZE 1         // Buffer Length
#define STRING_LEN 20

void clear_input_buffer()
{
    int ch;
    while ((ch = getchar()) != '\n' && ch != EOF)
        ;
}

static char get_command(char *string, size_t length)
{
    char chr = 0;
    size_t len = 0;

    clear_input_buffer();

    while (len < length)
    {
        chr = getchar();
        if (chr == '\n' || chr == '\r')
        {
            break;
        }
        else if ((chr >= 'a') && (chr <= 'z'))
        {
            putchar(chr);
            string[len] = chr;
            len++;
        }
        else
        {
            ;
        }
    }

    string[len] = '\0';

    if (strcmp(string, "red") == 0)
    {
        chr = 'R';
    }
    else if (strcmp(string, "blue") == 0)
    {
        chr = 'B';
    }
    else if (strcmp(string, "green") == 0)
    {
        chr = 'G';
    }
    else if (strcmp(string, "off") == 0)
    {
        chr = 'O';
    }
    else
    {
        chr = 'F';
    }

    return chr;
}

void app_main(void)
{
    i2c_master_bus_handle_t bus_handle;
    i2c_master_bus_config_t bus_config = {
        .i2c_port = MASTER,
        .sda_io_num = PIN_SDA,
        .scl_io_num = PIN_SCL,
        .clk_source = I2C_CLK_SRC_DEFAULT,
        .glitch_ignore_cnt = 7,
        .flags.enable_internal_pullup = true,
    };
    ESP_ERROR_CHECK(i2c_new_master_bus(&bus_config, &bus_handle));

    i2c_master_dev_handle_t dev_handle;
    i2c_device_config_t dev_config = {
        .dev_addr_length = I2C_ADDR_BIT_LEN_7,
        .device_address = SLAVE_ADDRESS,
        .scl_speed_hz = CLOCK_RATE,
    };
    ESP_ERROR_CHECK(i2c_master_bus_add_device(bus_handle, &dev_config, &dev_handle));

    for (int addr = 0; addr < 128; addr++)
    {
        if (ESP_OK == i2c_master_probe(bus_handle, addr, 50))
        {
            
        }
    }

    esp_task_wdt_deinit();

    uint8_t buffer[MSG_SIZE + 1] = {0};
    char string[STRING_LEN + 1];

    while (1)
    {

        printf("\nEnter the LED state: ");

        buffer[0] = get_command(string, STRING_LEN);

        if (buffer[0] != 'F')
        {
            if (ESP_OK == i2c_master_transmit(dev_handle, buffer, MSG_SIZE, -1))
            {
                if (ESP_OK == i2c_master_receive(dev_handle, buffer, MSG_SIZE, -1))
                {
                    if (buffer[0] == 'D')
                    {
                        printf(" => done");
                    }
                    else
                    {
                        printf(" => fail");
                    }
                }
                else
                {
                    printf(" => fail");
                }
            }
            else
            {
                printf(" => fail");
            }
        }
        else
        {
            printf(" => fail");
        }
        printf("\n");
    }
}
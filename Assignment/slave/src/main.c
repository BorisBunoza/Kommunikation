/**
 * @file main.c
 * @author Faroch Mehri (faroch.mehri@ya.se)
 * @brief An example of I2C controller in slave mode.
 *        Look at: https://docs.zephyrproject.org/latest/hardware/peripherals/i2c.html
 *                 https://docs.zephyrproject.org/latest/doxygen/html/group__i2c__interface.html
 *
 * @version 0.1
 * @date 2025-04-01
 *
 * @copyright Copyright (c) 2025
 *
 */
#include <ctype.h>
#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/i2c.h>
#include <zephyr/drivers/led.h>

#define MSG_SIZE 1
#define I2C_NODE DT_PROP(DT_PATH(zephyr_user), i2c)
#define ADDRESS DT_PROP(DT_PATH(zephyr_user), address)

static int count = 0;
static uint8_t buffer[MSG_SIZE];

/*
 * @brief Callback which is called when a write request is received from the master.
 * @param config Pointer to the target configuration.
 */
static int on_write_requested(struct i2c_target_config *config) { return 0; }

/*
 * @brief Callback which is called when a write is received from the master.
 * @param config Pointer to the target configuration.
 * @param val The byte received from the master.
 */
static int on_write_received(struct i2c_target_config *config, uint8_t val)
{
	const struct led_dt_spec red_dev = LED_DT_SPEC_GET(DT_NODELABEL(red_led));
	const struct led_dt_spec green_dev = LED_DT_SPEC_GET(DT_NODELABEL(green_led));
	const struct led_dt_spec blue_dev = LED_DT_SPEC_GET(DT_NODELABEL(blue_led));

	if (!led_is_ready_dt(&red_dev) || !led_is_ready_dt(&green_dev) || !led_is_ready_dt(&blue_dev))
	{
		printk("LED device is not ready");
		exit(EXIT_FAILURE);
	}
	
	bool status = true;

	if(val == 'R')
	{
		if ((0 != led_on_dt(&red_dev)) || (0 != led_off_dt(&green_dev)) || (0 != led_off_dt(&blue_dev)))
		{
			printk("Failed to turn the red LED on\n");
			status = false;
		}
	}
	else if(val == 'B')
	{
		if ((0 != led_off_dt(&red_dev)) || (0 != led_off_dt(&green_dev)) || (0 != led_on_dt(&blue_dev)))
		{
			printk("Failed to turn the blue LED on\n");
			status = false;
		}
	}
	else if (val == 'G')
	{
		if ((0 != led_off_dt(&red_dev)) || (0 != led_on_dt(&green_dev)) || (0 != led_off_dt(&blue_dev)))
		{
			printk("Failed to turn the green LED on\n");
			status = false;
		}
	}
	else if (val == 'O')
	{
		if ((0 != led_off_dt(&red_dev)) || (0 != led_off_dt(&green_dev)) || (0 != led_off_dt(&blue_dev)))
		{
			printk("Failed to turn off LED on\n");
			status = false;
		}
	}
	else
	{
		status = false;
	}

	if(status)
	{
		buffer[count] = 'D';
	}
	else
	{
		buffer[count] = 'F';
	}

	count = (count + 1) % MSG_SIZE;
	return 0;
}

/*
 * @brief Callback which is called when a read request is received from the master.
 * @param config Pointer to the target configuration.
 * @param val Pointer to the byte to be sent to the master.
 */
static int on_read_requested(struct i2c_target_config *config, uint8_t *val)
{
	*val = buffer[count];
	count = (count + 1) % MSG_SIZE;
	return 0;
}

/*
 * @brief Callback which is called when a read is processed from the master.
 * @param config Pointer to the target configuration.
 * @param val Pointer to the next byte to be sent to the master.
 */
static int on_read_processed(struct i2c_target_config *config, uint8_t *val)
{
	*val = buffer[count];
	count = (count + 1) % MSG_SIZE;
	return 0;
}

/*
 * @brief Callback which is called when the master sends a stop condition.
 * @param config Pointer to the target configuration.
 */
static int on_stop(struct i2c_target_config *config) { return 0; }

int main(void)
{
	const struct device *i2c_dev = DEVICE_DT_GET(I2C_NODE);

	if (!device_is_ready(i2c_dev))
	{
		printk("I2C device not found!\n");
		exit(EXIT_FAILURE);
	}

	const struct i2c_target_callbacks callbacks = {
		.write_requested = on_write_requested,
		.read_requested = on_read_requested,
		.write_received = on_write_received,
		.read_processed = on_read_processed,
		.stop = on_stop,
	};

	struct i2c_target_config config = {
		.address = ADDRESS,
		.callbacks = &callbacks,
	};

	if (0 != i2c_target_register(i2c_dev, &config))
	{
		printk("I2C target register failed\n");
		exit(EXIT_FAILURE);
	}

	while (1)
	{
		;
	}

	return 0;
}

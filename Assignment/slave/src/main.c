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

#define MSG_SIZE 8
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
	buffer[count] = toupper(val);
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
		k_msleep(K_FOREVER);
	}

	return 0;
}

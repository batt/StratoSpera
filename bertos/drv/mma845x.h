#ifndef DRV_MMA845X_H
#define DRV_MMA845X_H

#include <drv/i2c.h>
#include <cfg/compiler.h>

typedef enum Mma845xDataRate
{
	MMADR_800HZ,
	MMADR_400HZ,
	MMADR_200HZ,
	MMADR_100HZ,
	MMADR_50HZ,
	MMADR_12_5HZ,
	MMADR_6_25HZ,
	MMADR_1_56HZ,
	MMADR_CNT,
} Mma845xDataRate;

bool mma845x_read(I2c *i2c, uint8_t addr, float *acc);
bool mma845x_datarate(I2c *i2c, uint8_t addr, Mma845xDataRate rate);
bool mma845x_enable(I2c *i2c, uint8_t addr, bool state);

#endif /* DRV_MMA845X_H */

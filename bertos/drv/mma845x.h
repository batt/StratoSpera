#ifndef DRV_MMA845X_H
#define DRV_MMA845X_H

#include <drv/i2c.h>
#include <cfg/compiler.h>
#include <cpu/types.h>

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

typedef enum Mma845xDynamicRange
{
	MMADYN_2G,
	MMADYN_4G,
	MMADYN_8G,

	MMADYN_CNT,
} Mma845xDynamicRange;

typedef enum Mma845xAxis
{
	MMA_X,
	MMA_Y,
	MMA_Z,
	MMA_AXIZ_CNT,
} Mma845xAxis;

#define MMA_ERROR INT_MIN

int mma845x_read(I2c *i2c, uint8_t addr, Mma845xAxis axis);
bool mma845x_enable(I2c *i2c, uint8_t addr, bool state);
bool mma845x_init(I2c *i2c, uint8_t addr, Mma845xDynamicRange dyn_range);

#endif /* DRV_MMA845X_H */

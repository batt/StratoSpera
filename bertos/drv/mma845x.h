#ifndef DRV_MMA845X_H
#define DRV_MMA845X_H

#include <drv/i2c.h>
#include <cfg/compiler.h>

bool mma845x_read(I2c *i2c, uint8_t addr, float *acc);

#endif /* DRV_MMA845X_H */

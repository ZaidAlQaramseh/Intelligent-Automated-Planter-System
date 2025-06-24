#ifndef DS3231_H
#define DS3231_H

#include "mbed.h"

class DS3231 {
public:
    DS3231(I2C &i2c);
    bool readTime(struct tm *time);
    bool setTime(const struct tm *time);

private:
    I2C &_i2c;
    const int DS3231_ADDRESS = 0x68 << 1;

    char bcdToDec(char val);
    char decToBcd(char val);
};

#endif
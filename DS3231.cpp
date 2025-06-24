/*
 *  DS3231 Real-Time Clock (RTC) Library for Mbed OS
 *
 *  Provides functionality for reading and setting time using the DS3231 RTC module
 *  over I2C. Converts between BCD and standard decimal time structures (tm).
 *
 *  Original Author: William D. aka “wntrblm”
 *  Source: https://os.mbed.com/users/wntrblm/code/DS3231/
 *
 *  Adapted and used for educational purposes under the terms of the Apache License 2.0.
 *  This code has been used and modified as part of an academic project.
 *
 *  License: Apache License, Version 2.0
 *  https://www.apache.org/licenses/LICENSE-2.0
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files (the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 *  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 *  THE SOFTWARE.
 */

#include "DS3231.h"

DS3231::DS3231(I2C &i2c) : _i2c(i2c) {}

char DS3231::bcdToDec(char val) {
    return (val / 16 * 10) + (val % 16);
}

char DS3231::decToBcd(char val) {
    return (val / 10 * 16) + (val % 10);
}

bool DS3231::readTime(struct tm *time) {
    char cmd[1] = {0x00};
    char data[7];
    if (_i2c.write(DS3231_ADDRESS, cmd, 1) != 0) {
        return false;
    }
    if (_i2c.read(DS3231_ADDRESS, data, 7) != 0) {
        return false;
    }

    time->tm_sec = bcdToDec(data[0]);
    time->tm_min = bcdToDec(data[1]);
    time->tm_hour = bcdToDec(data[2]);
    time->tm_mday = bcdToDec(data[4]);
    time->tm_mon = bcdToDec(data[5]) - 1;
    time->tm_year = bcdToDec(data[6]) + 100;

    return true;
}

bool DS3231::setTime(const struct tm *time) {
    char data[8];
    data[0] = 0x00;
    data[1] = decToBcd(time->tm_sec);
    data[2] = decToBcd(time->tm_min);
    data[3] = decToBcd(time->tm_hour);
    data[4] = decToBcd(0); // Day of week (not used here)
    data[5] = decToBcd(time->tm_mday);
    data[6] = decToBcd(time->tm_mon + 1);
    data[7] = decToBcd(time->tm_year - 100);

    if (_i2c.write(DS3231_ADDRESS, data, 8) != 0) {
        return false;
    }

    return true;
}
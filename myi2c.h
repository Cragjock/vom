
#ifndef MY_I2C_H
#define MY_I2C_H

#include <iostream>     /// <contains string>? and uint8_t ???
///#include <iomanip>
///#include <cmath>
///#include <cstdint>
#include <unistd.h> /// for :: close
///#include <cerrno>
///#include <cstdio>
///#include <cstdlib>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>  /// for file ::open
///#include <time.h>
///#include <poll.h>
///#include <signal.h>
///#include <string>

#include <linux/swab.h>     /// for swab16 will this work ??
#include <byteswap.h>       /// placeholder to remember for ADS1015 code
//#include "ads1015.h"

#define swab16 __swab16 // http://lxr.free-electrons.com/source/include/linux/swab.h#L6
#define LBYTE(LB) (LB & 0x0FF)
#define HBYTE(HB) ((HB & 0xF00)>>8)


/// I2C slave address
#define I2C_SLAVE_ADDR_RTC      0x6f        /// TBD
#define I2C_SLAVE_ADDR_IMU      0x6b        /// Pololu IMU
#define I2C_SLAVE_ADDR_MAG      0x1e        /// Pololu mag
#define I2C_SLAVE_ADDR_RHTEMP   0x5f        /// MEMS HTS221
#define I2C_SLAVE_ADDR_PRESSURE 0x5c        /// MEMS LP25H
#define I2C_SLAVE_ADDR_PCA9685  0x40
#define I2C_SLAVE_ADDR_ADS1015  0x48
#define I2C_SLAVE_ADDR_MCP23008 0x20

#define READ_BUF_G         0x00
#define READ_BUF_XL        0x08

//class ads1015;

class I2CBus
{
    private:
        unsigned int i2cbus;
        unsigned int i2caddress;
        int ptrfile;
        char busfile[64];
        std::string i2cdev_name;
        int setup_device();

    public:
        I2CBus(unsigned int bus, unsigned int address);
        virtual int openi2c();
        virtual void closei2c();
        virtual ~I2CBus();

        /// std::unique_ptr<ads1015> ADSBus;

        int32_t myI2C_read_byte(int file);
        int myI2C_write_data(int file, uint8_t command_reg, uint8_t data);
        int myI2C_write_byte(int file, uint8_t data);
        int32_t myI2C_read_data(int file, uint8_t command);
        int myI2C_write_word_data(uint8_t command_reg, uint16_t data);


        int device_write_swap(uint8_t command_reg, uint16_t data);
        int16_t device_read_swap(uint8_t command);
        int16_t myI2C_read_word_data(uint8_t command);

        int device_read_block(int reg_request, int rd_size, unsigned char* readbuffer);
        int device_write_block(int reg_request, int rd_size, unsigned char* readbuffer);
        virtual int device_read(int reg_address);
        virtual int device_write(uint8_t command_reg, uint8_t data);
};

#endif // MY_I2C_H

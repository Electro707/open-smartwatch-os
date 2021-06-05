#ifndef RV8523_H
#define RV8523_h

#include <stdint.h>

#define RV8523_I2C_ADDRESS 0b1101000
#define RV8526_REG_CONTROL1 0x00
#define RV8526_REG_SECONDS 0x03

struct rv8523_time_struct {
    uint8_t seconds;
    uint8_t minutes;
    uint8_t hours;
    uint8_t date;
    uint8_t weekday;
    uint8_t month;
    uint8_t year; // The year is from 00 for 2000 to 99 for 2099
};

class RV8523{
    public:
        RV8523(void);
        void begin(void);
        void start(void);
        void get_time(struct rv8523_time_struct *time);


        /**
         * Functions which YOU must define
         */
        void i2c_write_one(uint8_t once_data);
        void (*i2c_write)(uint8_t *data, uint8_t len);
        void (*i2c_read)(int read_how_much, uint8_t *data_buff);
    
    private:
        void i2c_read_register(uint8_t start_reg, uint8_t len, uint8_t *data_buff);
};

#endif
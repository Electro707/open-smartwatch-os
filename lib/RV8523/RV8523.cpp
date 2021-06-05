#include "RV8523.h"

RV8523::RV8523(void){

}

void RV8523::start(void){
    uint8_t ret[1];
    i2c_read_register(RV8526_REG_CONTROL1, 1, ret);
    // Set the CONTROL1 register 
}

void RV8523::begin(void){
    uint8_t wr[2];
    // Set the CONTROL1 register to set to 24hour mode, and disable all interrupts 
    i2c_read_register(RV8526_REG_CONTROL1, 1, &wr[1]);
    wr[0] = RV8526_REG_CONTROL1;
    wr[1] &= ~(0b1111 << 0);
    i2c_write(wr, 2);
}

void RV8523::get_time(struct rv8523_time_struct *time){
    uint8_t rtc_ret[7];
    i2c_read_register(RV8526_REG_SECONDS, 7, rtc_ret);
    // Convert the times and write it in the struct
    time->seconds = bdc_to_dec(rtc_ret[0] & 0x7F);
    time->minutes = bdc_to_dec(rtc_ret[1]);
    time->hours = bdc_to_dec(rtc_ret[2]);
    time->date = bdc_to_dec(rtc_ret[3]);
    time->weekday = bdc_to_dec(rtc_ret[4]);
    time->month = bdc_to_dec(rtc_ret[5]);
    time->year = bdc_to_dec(rtc_ret[6]);
}

void RV8523::epoch_to_time(uint32_t epoch){

}

void RV8523::i2c_read_register(uint8_t start_reg, uint8_t len, uint8_t *data_buff){
    i2c_write_one(start_reg);
    i2c_read(len, data_buff);
}

/**
 * Wrapper function to write one data without having to define array in calling function
 */
void RV8523::i2c_write_one(uint8_t once_data){
    uint8_t d[] = {once_data};
    i2c_write(d, 1);
}

uint8_t bdc_to_dec(uint8_t bdc){
    return bdc - 6 * (bdc >> 4);
}
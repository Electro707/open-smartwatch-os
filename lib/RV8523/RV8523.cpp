#include "RV8523.h"

RV8523::RV8523(void){

}

void RV8523::start(void){
    uint8_t ret[2];
    i2c_read_register(RV8526_REG_CONTROL1, 1, &ret[1]);
    // Set the CONTROL1 register 
    ret[0] = RV8526_REG_CONTROL1;
    ret[1] &= ~(1 <<5);
    i2c_write(ret, 2);
}

void RV8523::begin(void){
    uint8_t wr[2];
    // Set the CONTROL1 register to set to 24hour mode, and disable all interrupts 
    i2c_read_register(RV8526_REG_CONTROL1, 1, &wr[1]);
    wr[0] = RV8526_REG_CONTROL1;
    wr[1] &= ~(0b1111 << 0);
    i2c_write(wr, 2);
    // Clear the second's clock integrity bit
    i2c_read_register(RV8526_REG_SECONDS, 1, &wr[1]);
    wr[0] = RV8526_REG_SECONDS;
    wr[1] &= ~(1 << 7);
    i2c_write(wr, 2);
}

void RV8523::get_time(struct rv8523_time_struct *time){
    uint8_t rtc_ret[7];
    i2c_read_register(RV8526_REG_SECONDS, 7, rtc_ret);
    // Convert the times and write it in the struct
    time->seconds = bcd_to_dec(rtc_ret[0] & 0x7F);
    time->minutes = bcd_to_dec(rtc_ret[1]);
    time->hours = bcd_to_dec(rtc_ret[2]);
    time->date = bcd_to_dec(rtc_ret[3]);
    time->weekday = bcd_to_dec(rtc_ret[4]);
    time->month = bcd_to_dec(rtc_ret[5]);
    time->year = bcd_to_dec(rtc_ret[6]);
}

void RV8523::set_time(struct rv8523_time_struct *time){
    uint8_t buff[8];
    i2c_read_register(RV8526_REG_SECONDS, 7, &buff[1]);
    buff[0] = RV8526_REG_SECONDS;
    buff[1] &= ~0x7F; buff[1] |= (dec_to_bcd(time->seconds) & 0x7F);
    buff[2] = dec_to_bcd(time->minutes);
    buff[3] = dec_to_bcd(time->hours);
    buff[4] = dec_to_bcd(time->date);
    // buff[5] = dec_to_bcd(time->weekday);
    buff[6] = dec_to_bcd(time->month);
    buff[7] = dec_to_bcd(time->year);
    i2c_write(buff, 8);
}

uint32_t RV8523::get_epoch_time(void){
    struct rv8523_time_struct t;
    get_time(&t);
    return from_time_to_epoch(&t);
}

void RV8523::set_epoch_time(uint32_t epoch){
    struct rv8523_time_struct t;
    from_epoch_to_time(epoch, &t);
    set_time(&t);
}

void RV8523::from_epoch_to_time(uint32_t epoch, struct rv8523_time_struct *t){
    epoch = epoch - 946684800;
    t->year = epoch / 31556926; epoch = epoch % 31556926;
    t->month = epoch / 2629743; epoch = epoch % 2629743;
    t->date = epoch / 86400; epoch = epoch % 86400;
    t->hours = epoch / 3600; epoch = epoch % 3600;
    t->minutes = epoch / 60; epoch = epoch % 60;
    t->seconds = epoch; 
}

uint32_t RV8523::from_time_to_epoch(struct rv8523_time_struct *t){
    uint32_t e = 946684800;
    e += t->seconds;
    e += t->minutes * 60;
    e += t->hours * 3600;
    e += t->date * 86400;
    e += t->month * 2629743;
    e += t->year * 31556926;
    return e;
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

uint8_t RV8523::bcd_to_dec(uint8_t bdc){
    return bdc - 6 * (bdc >> 4);
}

uint8_t RV8523::dec_to_bcd(uint8_t dec){
    return( ((dec/10) << 4) | (dec%10) );
}
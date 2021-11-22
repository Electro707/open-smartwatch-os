#include "IT7259.h"

// Based off example code from https://www.buydisplay.com/240x240-round-ips-tft-lcd-display-1-28-inch-capactive-touch-circle-screen
// Need to refractor some things later on

#define IT7259_ADDRESS 0x8C >> 1

#define COMMAND_BUFFER_INDEX   				0x20 
#define QUERY_BUFFER_INDEX						0x80
#define COMMAND_RESPONSE_BUFFER_INDEX 0xA0 
#define POINT_BUFFER_INDEX    				0xE0 
#define QUERY_SUCCESS     						0x00 
#define QUERY_BUSY     								0x01 
#define QUERY_ERROR     							0x02 
#define QUERY_POINT     							0x80 

IT7259::IT7259(int reset_pin){
    _reset_pin = reset_pin;
    pinMode(_reset_pin, OUTPUT);
}

void IT7259::init(void){
    digitalWrite(_reset_pin, LOW);
    delay(20);
    digitalWrite(_reset_pin, HIGH);
    delay(500);


}

IT7259_Touch IT7259::read_touch_point(void){
    IT7259_Touch ret;
    ret.is_touch = false;
    uint8_t pointdata[14];
    if(read_memory_buffer(POINT_BUFFER_INDEX, pointdata, 5)) return ret;
    if(pointdata[0] & 0x08)													//point
    {
        ret.x1 = ((pointdata[3] & 0x0F) << 8) + pointdata[2];
        ret.y1 = ((pointdata[3] & 0xF0) << 4) + pointdata[4];
        ret.is_touch = true;
    }

    return ret;
}

int IT7259::enable_interrupt_on_touch(void){
    uint8_t out_data[4] = {0x02, 0x04, 1, 0};
    uint8_t ret[2];
    if(check_if_busy()){return 1;}
    if(!write_memory_buffer(COMMAND_BUFFER_INDEX, out_data, 4)) return 1;

    if(check_if_busy()){return 1;}
    if(!read_memory_buffer(COMMAND_RESPONSE_BUFFER_INDEX, ret, 2)) return 1;

    if(ret[0] == 0){
        return 0;
    }
    return 1;
}

int IT7259::identify_sensor(void){
    uint8_t cmd = 0;
    uint8_t read_arr[10];

    if(check_if_busy()){return 1;}
    if(write_memory_buffer(COMMAND_BUFFER_INDEX, &cmd, 1)) return 1;
    
    if(check_if_busy()){return 1;}
	if(read_memory_buffer(COMMAND_RESPONSE_BUFFER_INDEX, read_arr, 10)) return 1;

    if(read_arr[1] == 'I' && read_arr[2] == 'T' && read_arr[3] == 'E' && read_arr[4] == '7' && read_arr[5] == '2'  && read_arr[6] == '6'  && read_arr[7] == '0')
		return 1;
    return 0;
}

int IT7259::check_if_busy(void){
    uint8_t read_buff[2];
    int i=0;
    do{
        read_memory_buffer(QUERY_BUFFER_INDEX, read_buff, 1);
        Serial.println(read_buff[0]);
        if(++i == 500){
            return 1;
        }
    }while(read_buff[0] & QUERY_BUSY);
    return 0;
}

int IT7259::write_memory_buffer(int addr, uint8_t *write_buff, int len){
    Wire.beginTransmission(IT7259_ADDRESS);
    Wire.write(addr);
    for(int i=0;i<len;i++){
        Wire.write(write_buff[i]);
    }   
    Wire.endTransmission();
    return 0;
}

int IT7259::read_memory_buffer(int addr, uint8_t *read_buff, int len){
    int i=0;
    Wire.beginTransmission(IT7259_ADDRESS);
    Wire.write(addr);
    Wire.endTransmission(false);
    Wire.requestFrom(IT7259_ADDRESS, len);
    while(Wire.available()){
        read_buff[i] = Wire.read();
        i++;
    }
    return 0;
}
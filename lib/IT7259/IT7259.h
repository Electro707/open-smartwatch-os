#ifndef IT7259_H
#define IT7259_H

#include <Arduino.h>
#include <Wire.h>

struct IT7259_Touch{
    unsigned int x1 = 0;
    unsigned int y1 = 0;
    bool is_touch = false;
    bool is_hold = false;
};

class IT7259 {
  public:
    IT7259(int reset_pin);
    void init(void);
    int check_if_busy(void);
    int identify_sensor(void);
    IT7259_Touch read_touch_point(void);
    int enable_interrupt_on_touch(void);
  
  protected:
    int read_memory_buffer(int addr, uint8_t *read_buff, int len);
    int write_memory_buffer(int addr, uint8_t *write_buff, int len);
  
  private:
    int _reset_pin;
};

#endif
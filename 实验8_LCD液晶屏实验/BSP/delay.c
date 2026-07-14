#include "delay.h"

/*****  延时函数,ms,us  *****/
void delay_us(uint16_t nus){
    uint16_t i;
    while(nus--){
        i = 31;         //约1us延时
        while(i--){};
    }
}

void delay_ms(u16 nms){
    uint16_t i;
    while(nms--){
        i = 33800;       //约1ms延时
        while(i--){};
    }
}

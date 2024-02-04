#include "motor_control.h"
#include <pigpio.h>

//Drive forward
void Driving_forward(int in1, int in2, int in3, int in4, int speed)
{
    //in1 and in2 are a set of signals on the wheel, so high or low signals mus>
    gpioWrite(in1, 1);    //right side's in1 signal is low
    gpioWrite(in2, 0);    //right side's in2 signal is high
    gpioWrite(in3, 0);    //left side's in3 signal is low
    gpioWrite(in4, 1);    //left side's in4 signal is high
    gpioPWM(ENA, speed);    //enableA takes a duty
    gpioPWM(ENB, speed);    //enableB takes a duty
}

//Drive backward
void Driving_backward(int in1, int in2, int in3 ,int in4, int speed)
{
    gpioWrite(in1, 0);
    gpioWrite(in2, 1);
    gpioWrite(in3, 1);
    gpioWrite(in4, 0);
    gpioPWM(ENA, speed);
    gpioPWM(ENB, speed);
}

void Driving_left(int in1, int in2, int in3, int in4, int speed)
{
    gpioWrite(in1, 0);
    gpioWrite(in2, 1);
    gpioWrite(in3, 0);
    gpioWrite(in4, 1);
    gpioPWM(ENA, speed);
    gpioPWM(ENB, speed);
}

//Drive stop so i will set the speed 0.
void Driving_stop(int in1, int in2, int in3, int in4, int speed)
{
    gpioWrite(in1, 1);    //right side's in1 signal is high
    gpioWrite(in2, 1);    //right side's in1 signal is high
    gpioWrite(in3, 1);    //left side's in4 signal is high
    gpioWrite(in4, 1);    //left side's in4 signal is high, this direction is N>
    gpioPWM(ENA, speed);    //0
    gpioPWM(ENB, speed);    //0
}


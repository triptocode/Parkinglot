#include <pigpio.h>
#include <iostream>
#include <csignal>
#include "motor_control.h"
#include <unistd.h>

// define signal handler function
void signalHandler(int signum) {
    std::cout << "Interrupt signal (" << signum << ") received.\n";
    // you can exit or perform additional cleanup
    gpioTerminate();
    exit(signum);
}

// function for bluetooth input's processing
void controlMotors(char input) {
    // Add your motor control logic here
    switch (input) {

        case '0':
            std::cout << "Driving backward" << std::endl;
            Driving_backward(IN1, IN2, IN3, IN4, 70);
            usleep(520000);
            break;
        case '1':
            std::cout << "Driving forward" << std::endl;
            Driving_forward(IN1, IN2, IN3, IN4, 70);
            usleep(500000);
            break;
        case '2':
            std::cout << "Driving fast" << std::endl;
            Driving_forward(IN1, IN2, IN3, IN4, 90);
            usleep(1380000);
            break;
        case '3':
            std::cout << "Driving forward" << std::endl;
            Driving_forward(IN1, IN2, IN3, IN4, 70);
            usleep(1150000);
            break;
        case '4':
            std::cout << "Driving forward" << std::endl;
            Driving_forward(IN1, IN2, IN3, IN4, 70);
            usleep(1500000);
            break;
        default:
            Driving_stop(IN1, IN2, IN3, IN4, 0);
            break;
    }
}


int main() {
    // gpio init
    if (gpioInitialise() < 0) {
        std::cerr << "GPIO initialization failed. Exiting..." << std::endl;
        return 1;
    }

    signal(SIGINT, signalHandler);

    const char* device = "/dev/ttyS0";
    const int baudrate = 9600;

    char* mutableDevice = const_cast<char*>(device);
    int serial_port = serOpen(mutableDevice, baudrate, 0);
    if (serial_port < 0) {
        std::cerr << "Failed to open serial port. Exiting..." << std::endl;
        gpioTerminate();
        return 1;
    }

    // motor, pwm set
    gpioSetMode(IN1, PI_OUTPUT);
    gpioSetMode(IN2, PI_OUTPUT);
    gpioSetMode(IN3, PI_OUTPUT);
    gpioSetMode(IN4, PI_OUTPUT);
    gpioSetMode(ENA, PI_OUTPUT);
    gpioSetMode(ENB, PI_OUTPUT);
    gpioSetPWMfrequency(ENA, 50);
    gpioSetPWMfrequency(ENB, 50);

    while (1) {
        // reading input
        if (serDataAvailable(serial_port) > 0) {
            char input = serReadByte(serial_port);
            // bletooth input processing
            controlMotors(input);
        }
        usleep(100000); // 0.1sec waiting
    }

    serClose(serial_port);
    gpioTerminate();

    return 0;
}

/* 
 * File:   WiringPiPin.hpp
 * Author: markus
 *
 * Created on 25. October 2016, 21:26
 */

#ifndef WIRING_PI_PIN_HPP
#define	WIRING_PI_PIN_HPP

/** Represents a GPIO pin of Raspberry PI revision 2. */
enum class GpioPin {
    GPIO_0,
    GPIO_1,
    GPIO_2,
    GPIO_3,
    GPIO_4,
    GPIO_5,
    GPIO_6,
    GPIO_7,
    GPIO_21,
    GPIO_22,
    GPIO_23,
    GPIO_24,
    GPIO_25,
    GPIO_26,
    GPIO_27,
    GPIO_28,
    GPIO_29
};

int getWiringPiPin(GpioPin gpioPin) {
    switch (gpioPin) {
        case GpioPin::GPIO_0:
            return 0;
        case GpioPin::GPIO_1:
            return 1;
        case GpioPin::GPIO_2:
            return 2;
        case GpioPin::GPIO_3:
            return 3;
        case GpioPin::GPIO_4:
            return 4;
        case GpioPin::GPIO_5:
            return 5;
        case GpioPin::GPIO_6:
            return 6;
        case GpioPin::GPIO_7:
            return 7;
        case GpioPin::GPIO_21:
            return 21;
        case GpioPin::GPIO_22:
            return 22;
        case GpioPin::GPIO_23:
            return 23;
        case GpioPin::GPIO_24:
            return 24;
        case GpioPin::GPIO_25:
            return 25;
        case GpioPin::GPIO_26:
            return 26;
        case GpioPin::GPIO_27:
            return 27;
        case GpioPin::GPIO_28:
            return 28;
        case GpioPin::GPIO_29:
            return 29;
        default:
            return 0;
    }
};

GpioPin getGpioPin(int wiringPiPin) {
    switch (wiringPiPin) {
        case 0:
            return GpioPin::GPIO_0;
        case 1:
            return GpioPin::GPIO_1;
        case 2:
            return GpioPin::GPIO_2;
        case 3:
            return GpioPin::GPIO_3;
        case 4:
            return GpioPin::GPIO_4;
        case 21:
            return GpioPin::GPIO_21;
        case 22:
            return GpioPin::GPIO_22;
        case 23:
            return GpioPin::GPIO_23;
        case 24:
            return GpioPin::GPIO_24;
        case 25:
            return GpioPin::GPIO_25;
        case 26:
            return GpioPin::GPIO_26;
        case 27:
            return GpioPin::GPIO_27;
        case 28:
            return GpioPin::GPIO_28;
        case 29:
            return GpioPin::GPIO_29;
        default:
            return GpioPin::GPIO_0;
    }
};

#endif	/* WIRING_PI_PIN_HPP */


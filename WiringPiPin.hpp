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
    GPIO_2,
    GPIO_3,
    GPIO_4,
    GPIO_5,
    GPIO_6,
    GPIO_7,
    GPIO_8,
    GPIO_9,
    GPIO_10,
    GPIO_11,
    GPIO_12,
    GPIO_13,
    GPIO_14,
    GPIO_15,
    GPIO_16,
    GPIO_17,
    GPIO_18,
    GPIO_19,
    GPIO_20,
    GPIO_21,
    GPIO_22,
    GPIO_23,
    GPIO_24,
    GPIO_25,
    GPIO_26,
    GPIO_27
};

int getWiringPiPin(GpioPin gpioPin) {
    switch (gpioPin) {
        case GpioPin::GPIO_2:
            return 8;
        case GpioPin::GPIO_3:
            return 9;
        case GpioPin::GPIO_4:
            return 7;
        case GpioPin::GPIO_5:
            return 21;
        case GpioPin::GPIO_6:
            return 22;
        case GpioPin::GPIO_7:
            return 11;
        case GpioPin::GPIO_8:
            return 10;
        case GpioPin::GPIO_9:
            return 13;
        case GpioPin::GPIO_10:
            return 12;
        case GpioPin::GPIO_11:
            return 14;
        case GpioPin::GPIO_12:
            return 26;
        case GpioPin::GPIO_13:
            return 23;
        case GpioPin::GPIO_14:
            return 15;
        case GpioPin::GPIO_15:
            return 16;
        case GpioPin::GPIO_16:
            return 27;
        case GpioPin::GPIO_17:
            return 0;
        case GpioPin::GPIO_18:
            return 1;
        case GpioPin::GPIO_19:
            return 24;
        case GpioPin::GPIO_20:
            return 28;
        case GpioPin::GPIO_21:
            return 29;
        case GpioPin::GPIO_22:
            return 3;
        case GpioPin::GPIO_23:
            return 4;
        case GpioPin::GPIO_24:
            return 5;
        case GpioPin::GPIO_25:
            return 6;
        case GpioPin::GPIO_26:
            return 25;
        case GpioPin::GPIO_27:
            return 2;
        default:
            return 8;
    }
};

GpioPin getGpioPin(int wiringPiPin) {
    switch (wiringPiPin) {
        case 8:
            return GpioPin::GPIO_2;
        case 9:
            return GpioPin::GPIO_3;
        case 7:
            return GpioPin::GPIO_4;
        case 21:
            return GpioPin::GPIO_5;
        case 22:
            return GpioPin::GPIO_6;
        case 11:
            return GpioPin::GPIO_7;
        case 10:
            return GpioPin::GPIO_8;
        case 13:
            return GpioPin::GPIO_9;
        case 12:
            return GpioPin::GPIO_10;
        case 14:
            return GpioPin::GPIO_11;
        case 26:
            return GpioPin::GPIO_12;
        case 23:
            return GpioPin::GPIO_13;
        case 15:
            return GpioPin::GPIO_14;
        case 16:
            return GpioPin::GPIO_15;
        case 27:
            return GpioPin::GPIO_16;
        case 0:
            return GpioPin::GPIO_17;
        case 1:
            return GpioPin::GPIO_18;
        case 24:
            return GpioPin::GPIO_19;
        case 28:
            return GpioPin::GPIO_20;
        case 29:
            return GpioPin::GPIO_21;
        case 3:
            return GpioPin::GPIO_22;
        case 4:
            return GpioPin::GPIO_23;
        case 5:
            return GpioPin::GPIO_24;
        case 6:
            return GpioPin::GPIO_25;
        case 25:
            return GpioPin::GPIO_26;
        case 2:
            return GpioPin::GPIO_27;
        default:
            return GpioPin::GPIO_2;
    }
};

#endif	/* WIRING_PI_PIN_HPP */


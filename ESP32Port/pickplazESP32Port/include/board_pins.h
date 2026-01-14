#ifndef PICKPLAZ_BOARD_PINS_H_
#define PICKPLAZ_BOARD_PINS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <stdint.h>

#define BOARD_GPIO_UNUSED (-1)

enum {
    BOARD_GPIO_LED0 = 0,
    BOARD_GPIO_LED1 = 1,
    BOARD_GPIO_LED2 = 3,
    BOARD_GPIO_LED3 = 5,
    BOARD_GPIO_MOTOR_IN1 = 6,
    BOARD_GPIO_MOTOR_IN2 = 7,
    BOARD_GPIO_BUTTON_FWD = 20,
    BOARD_GPIO_BUTTON_REV = 21,
    BOARD_GPIO_OPTO_INT = 4
};

#define BOARD_BUTTON_ACTIVE_LOW 1

#ifdef __cplusplus
}
#endif

#endif

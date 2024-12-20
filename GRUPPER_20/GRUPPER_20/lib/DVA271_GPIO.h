// DVA271_GPIO.h
// Ali Haj Marsak, Ayman Alhomsi
#ifndef DVA271_GPIO_H
#define DVA271_GPIO_H

#include <gpiod.h>

// Definiera GPIO-pinnar
#define DATA_PIN    17
#define LATCH_PIN   27
#define CLOCK_PIN   22
#define LED1_PIN    23
#define LED2_PIN    24
#define LED3_PIN    4
#define LED4_PIN    26

#define CHIP_NAME "/dev/gpiochip0"

// Initialiserar HC595 och GPIO-pinnar
int hc595_init(void);

// Indikerar temperaturen via HC595
int temp_indicate(float temperature);

// Flippar tillståndet för en LED-pin
int flip_pin(int pin);

// Sätter tillståndet för båda LEDs
int set_led_state(int led1, int led2);


#endif // DVA271_GPIO_H

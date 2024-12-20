// Ali Haj Marsak, Ayman Alhomsi
// DVA271_GPIO.c

#include "DVA271_GPIO.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

// Deklaration av statiska variabler
static struct gpiod_chip *chip = NULL; // anger vilken chip
static struct gpiod_line *data_line = NULL; // linje för data
static struct gpiod_line *latch_line = NULL; // linje för latch 
static struct gpiod_line *clock_line = NULL; // linje för klocka 
static struct gpiod_line *led1_line = NULL; // första led 
static struct gpiod_line *led2_line = NULL; // andra led 


int temp_indicate(float temperature) { //indikerar tempratur genom att skicka data till HC595
    if (!data_line || !latch_line || !clock_line) { // kontroll om linjerna är initialiserade 
        fprintf(stderr, "GPIO-linjer är inte initialiserade.\n"); 
        return 1;
    } 

    unsigned int temp_int = (unsigned int)temperature; // konvertera till heltal 
    for (int i = 0; i < 8; i++) { // loopar för att skicka 8 bitar 
        int bit = (temp_int >> i) & 1; //extraherar biten 
        if (gpiod_line_set_value(data_line, bit) < 0) { // sätt ett värde till data linjen 
            perror("Fel vid inställning av data_line");
            return 1;
        }
        if (gpiod_line_set_value(clock_line, 1) < 0) { // gör clock line till high
            perror("Fel vid inställning av clock_line till HIGH");
            return 1;
        }
        usleep(1000);
        if (gpiod_line_set_value(clock_line, 0) < 0) { // clock line till low 
            perror("Fel vid inställning av clock_line till LOW");
            return 1;
        }
    }

    // Latchar datan för att lagra data 
    if (gpiod_line_set_value(latch_line, 1) < 0) { // latch line till 1 
        perror("Fel vid inställning av latch_line till HIGH");
        return 1;
    }
    usleep(1000);
    if (gpiod_line_set_value(latch_line, 0) < 0) { // latch till 0 
        perror("Fel vid inställning av latch_line till LOW");
        return 1;
    }

    return 0;
}


int set_led_state(int led1, int led2) { //styr tillståndet för led 1 och 2 
    if (!led1_line || !led2_line) { // kontroll för initaliseringen 
        fprintf(stderr, "Fel: LED-linjer är inte initialiserade.\n"); 
        return 1;
    }

    if (gpiod_line_set_value(led1_line, led1) < 0) { // led value till led 1 
        perror("Fel vid inställning av värde för LED1");
        return 1;
    }

    if (gpiod_line_set_value(led2_line, led2) < 0) { // value för led 2 
        perror("Fel vid inställning av värde för LED2");
        return 1;
    }

    return 0;
}



int hc595_init(void) { //initierar GPIO och konfig för HC595
    chip = gpiod_chip_open(CHIP_NAME); // öponar chippet 
    if (!chip) {
        perror("Fel vid öppning av GPIO-chip");
        return 1;
    }
 // hämtar linjer för gpio 
    data_line = gpiod_chip_get_line(chip, DATA_PIN);
    latch_line = gpiod_chip_get_line(chip, LATCH_PIN);
    clock_line = gpiod_chip_get_line(chip, CLOCK_PIN);
    led1_line = gpiod_chip_get_line(chip, LED1_PIN);
    led2_line = gpiod_chip_get_line(chip, LED2_PIN);

    if (!data_line || !latch_line || !clock_line || !led1_line || !led2_line) { // om inte iniliserade 
        perror("Kunde inte hämta GPIO-linjer");
        gpiod_chip_close(chip); // stäng av 
        return 1;
    }

// konfigurerar linjerna till utgångar 
    if (gpiod_line_request_output(data_line, "HC595", 0) < 0 ||
        gpiod_line_request_output(latch_line, "HC595", 0) < 0 ||
        gpiod_line_request_output(clock_line, "HC595", 0) < 0 ||
        gpiod_line_request_output(led1_line, "LED1", 0) < 0 ||
        gpiod_line_request_output(led2_line, "LED2", 0) < 0) {
        perror("Kunde inte begära GPIO-utgång");
        gpiod_chip_close(chip);
        return 1;
    }

    return 0;
}

// flippar tillståndet på en specifik gpio pin 
int flip_pin(int pin) {
    struct gpiod_line *line = NULL;
 // identifiera vilka linjera som motsvarar vilka pinnar 
    if (pin == LED1_PIN) {
        line = led1_line;
    } else if (pin == LED2_PIN) {
        line = led2_line;
    } else {
        fprintf(stderr, "Ogiltigt pin-nummer: %d\n", pin);
        return 1;
    }

    if (!line) { // kontroll på initialiseringen 
        fprintf(stderr, "GPIO-linje är inte initialiserad för pin: %d\n", pin);
        return 1;
    }

    int value = gpiod_line_get_value(line);  // läs värdet från linjen 
    if (value < 0) {
        perror("Fel vid hämtning av värde från GPIO-linje");
        return 1;
    }

    if (gpiod_line_set_value(line, !value) < 0) { // växla värdet
        perror("Fel vid inställning av värde på GPIO-linje");
        return 1;
    }

    return 0;
}
// Ali Haj Marsak, Ayman Alhomsi
// DVA271_GPIO.c

#include "DVA271_GPIO.h"
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>

#define TEMP_FILE "/sys/class/thermal/thermal_zone0/temp"

// Deklaration av statiska variabler
static struct gpiod_chip *chip = NULL; // anger vilken chip
static struct gpiod_line *data_line = NULL; // linje för data
static struct gpiod_line *latch_line = NULL; // linje för latch 
static struct gpiod_line *clock_line = NULL; // linje för klocka 
static struct gpiod_line *led1_line = NULL; // första led 
static struct gpiod_line *led2_line = NULL; // andra led 
static struct gpiod_line *led3_line = NULL; // första led 
static struct gpiod_line *led4_line = NULL; // andra led 


int hc595_shift_out(int value) {
    // Shift out 8 bits to the shift register using Data, Clock, and Latch lines
    for (int i = 7; i >= 0; i--) {
        int bit_val = (value >> i) & 1;

        // Set data line according to the current bit
        gpiod_line_set_value(data_line, bit_val);

        // Pulse the clock line
        gpiod_line_set_value(clock_line, 1);
        usleep(1);
        gpiod_line_set_value(clock_line, 0);
    }

    // Pulse the latch line to transfer the shifted data to the output pins
    gpiod_line_set_value(latch_line, 1);
    usleep(1);
    gpiod_line_set_value(latch_line, 0);

    return 0;
}

int temp_indicate() {
    FILE *temp_file;
    char temp_str[16];
    int temp_value;

    // Öppnar filen som innehåller CPU-temperaturen.
    temp_file = fopen(TEMP_FILE, "r");
    if (!temp_file) {
        perror("Failed to open temperature file");
        return 1; // Fel vid öppning av fil.
    }

    // Läser temperaturvärdet som en sträng.
    if (!fgets(temp_str, sizeof(temp_str), temp_file)) {
        perror("Failed to read temperature");
        fclose(temp_file);
        return 1; // Fel vid läsning av temperatur.
    }
    fclose(temp_file);

    // Konverterar strängen till Celsiusvärde.
    temp_value = atoi(temp_str) / 1000; // Temperaturen i °C.
    printf("Current CPU temperature: %d°C\n", temp_value);

    // Beräkna antal LED-lampor som ska lysa beroende på temperaturen
    // Exempel: Varje 10°C representerar en extra LED.
    // Om temp_value är 85°C eller mer, alla 8 tänds.
    int led_count = temp_value / 10;
    if (led_count > 8) {
        led_count = 8;
    }

    // Skapa ett mönster för LED-lamporna.
    // Om led_count = 3, betyder det att 3 lägsta bitarna ska vara 1 (00000111).
    // (1 << led_count) - 1 skapar ett sådant maskeringsvärde.
   int led_pattern = 0;
    if (led_count > 0) {
        led_pattern = (1 << led_count) - 1;
    }

    // Skicka ut bitmönstret till 74HC595
    hc595_shift_out(led_pattern);

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
    led3_line = gpiod_chip_get_line(chip, LED3_PIN);
    led4_line = gpiod_chip_get_line(chip, LED4_PIN);

    if (!data_line || !latch_line || !clock_line || !led1_line || !led2_line || !led3_line || !led4_line) { // om inte iniliserade 
        perror("Kunde inte hämta GPIO-linjer");
        gpiod_chip_close(chip); // stäng av 
        return 1;
    }

// konfigurerar linjerna till utgångar 
    if (gpiod_line_request_output(data_line, "HC595", 0) < 0 ||
        gpiod_line_request_output(latch_line, "HC595", 0) < 0 ||
        gpiod_line_request_output(clock_line, "HC595", 0) < 0 ||
        gpiod_line_request_output(led1_line, "LED1", 0) < 0 ||
        gpiod_line_request_output(led3_line, "LED1", 0) < 0 ||
      gpiod_line_request_output(led4_line, "LED1", 0) < 0 ||
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

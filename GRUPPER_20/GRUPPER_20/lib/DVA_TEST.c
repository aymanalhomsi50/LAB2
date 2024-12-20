// DVA_TEST.c
 // Ali Haj Marsak ,Ayman Alhomsi 

#include "DVA_TEST.h"            // Inkludera egen headerfil för deklarationer
#include "DVA271_GPIO.h"        // Inkludera GPIO-hanteringsheaderfilen för LED-kontroll
#include <stdio.h>              // Inkludera standard I/O-bibliotek för printf och fprintf
#include <unistd.h>             // Inkludera unistd.h för usleep-funktionen


int test_leds() {                // Definition av test_leds-funktionen
    const int antal_blink = 20;  // Antal gånger LEDs ska blinka
    const useconds_t blink_interval = 250000; // Tiden mellan blinks i mikrosekunder (250 ms)

    for (int i = 0; i < antal_blink; i++) { // För-loop som itererar antal_blink gånger
        // Tänd LED:arna
        if (set_led_state(1, 1) != 0) {     // Anropa set_led_state för att tända LEDs, kontrollera om det misslyckas
            fprintf(stderr, "Fel vid tändning av LEDs vid iteration %d\n", i + 1); // Skriv felmeddelande till standardfelström
            return 1;                        // Returnera 1 för att indikera fel
        }

        // Vänta
        usleep(blink_interval);             // Pausa programmet i blink_interval mikrosekunder (250 ms)

        // Släck LED:arna
        if (set_led_state(0, 0) != 0) {     // Anropa set_led_state för att släcka LEDs, kontrollera om det misslyckas
            fprintf(stderr, "Fel vid släckning av LEDs vid iteration %d\n", i + 1); // Skriv felmeddelande till standardfelström
            return 1;                        // Returnera 1 för att indikera fel
        }

        // Vänta
        usleep(blink_interval);             // Pausa programmet i blink_interval mikrosekunder (250 ms)
    }

    printf("LED-test genomfört framgångsrikt.\n"); // Skriv framgångsmeddelande till standardutström
    return 0;                                        // Returnera 0 för att indikera framgång
}

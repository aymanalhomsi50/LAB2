 // Ali Haj Marsak, Ayman Alhomsi
#include "DVA271_EEPROM.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/i2c.h>

static int file; // Filhanterare för I2C bus

// Base address of the first EEPROM block (0x50)
#define BASE_ADDR_EEPROM 0x50

static int eeprom_wait_ready(unsigned char chip_address) { // används för att vänta tills eeprom är redo att skriva
// EEPROM skickar ett ACK som betyder att den är redo 
    for (int i = 0; i < 100; i++) { //
        if (ioctl(file, I2C_SLAVE, chip_address) < 0) {
            perror("Fel kan inte sätta I2C address");
            return -1;
        }

        if (write(file, NULL, 0) >= 0) { // om det lyckas svarar EEPROM med ACK, vilket betyder redo för ny skrivning
            
            return 0; // klart 
        }
        usleep(5000); // ge tid för eeprom tills den är redo 
    }
    fprintf(stderr, "EEPROM inte redo än");
    return -1; // EEPROM inte redo för en skrivning
}

// funktion som bestämmer vilket chip samt address det ska vara på.
static int select_chip_and_address(unsigned short full_address, unsigned char *chip_address, unsigned char *internal_addr) {
    if (full_address >= EEPROM_SIZE) { //kontroll om addressen ligger i eeproms storlek
        fprintf(stderr, "Addressen är utanför eeproms intervall"); 
        return -1;
    }

    int block = full_address / BLOCK_SIZE; // Beräknar vilket block och adress som motsvara adressen
    *chip_address = BASE_ADDR_EEPROM + block; // chip adressen baseras på blocknumret
    *internal_addr = full_address % BLOCK_SIZE; // intern adress är resten 
    return 0;
}

// Funktionen som skriver data till EPPROM via en adress 
static int eeprom_write(unsigned short full_address, const unsigned char *data, int length) {
    unsigned char chip_address;
    unsigned char internal_addr;
    if (select_chip_and_address(full_address, &chip_address, &internal_addr) < 0) { //Välj chip och adress
        return -1; 
    }

    if (ioctl(file, I2C_SLAVE, chip_address) < 0) { //ioctl används för konfig av filbeskrivaren för att kommunicera med slave enheten
     
     //I2C_SLAVE är enheten som tar emot data och kommando från master enheten (eeprom slav)
        perror("Kan ej sätta I2C slave adress ");
        return -1;
    }

    unsigned char buffer[1 + length]; // skapar buffer som har en adress samt data
    buffer[0] = internal_addr; // Intern adress först i buffert 
    memcpy(&buffer[1], data, length); // kopiera från data till buffert

    if (write(file, buffer, 1 + length) != (1 + length)) { // skickar bufferten till epprom och kollar på resultatet
        perror("Kan ej skriva till i2c bus"); 
        return -1;
    }

// väntar tills eeprom är klar med tidigare skrivning 
    if (eeprom_wait_ready(chip_address) != 0) {
        return -1;
    }

    return 0; // skrivningen är klar 
}

// Funktionen som läser data från eeprom vid en adress 
static int eeprom_read(unsigned short full_address, unsigned char *data, int length) {
    unsigned char chip_address;
    unsigned char internal_addr;
    if (select_chip_and_address(full_address, &chip_address, &internal_addr) < 0) { // väljer chip och adress
        return -1; // om inte i intervallen så är det fel 
    }

    if (ioctl(file, I2C_SLAVE, chip_address) < 0) { // väljer en slave adress för operationen  om det är mindre än 0 
        perror("Fel med att sätta  I2C slave address"); // fel meddlande 
        return -1;
    }


// skickar interna adressen till eeprom 
    unsigned char addr_buf = internal_addr;
    if (write(file, &addr_buf, 1) != 1) {
        perror("Kan inte hitta ett address"); 
        return -1;
    }

    // läs data från eeprom och kontrollerar resultatet
    if (read(file, data, length) != length) {
        perror("Kan ej läsa från EEPROM");
        return -1;
    }

    return 0;
}

int eeprom_setup() { // initiera I2C kommunikation
    char *bus = "/dev/i2c-1"; // anger vilken I2C-byss soom ska används 
    if ((file = open(bus, O_RDWR)) < 0) { 
        perror("Kan inte öppna bussen");
        return 1;
    }
    return 0;
}

int get_joke(int number, char **ptr) { // läser en joke från eeprom 
    if (number < 0) return -1; //ogiltigt index

    // Beräkna startadress för skämtet 
    unsigned short address = number * 255;
    if (address + 255 > EEPROM_SIZE) {
        printf("Fel, finns ej i intervallet ");
        return -1;
    }

    unsigned char buffer[255]; //Buffert som lagrar skämt
    if (eeprom_read(address, buffer, 255) != 0) {
        printf("Skämtet går inte att läsa\n"); //
        return -1;
    }

    *ptr = malloc(256); //allokera minnet för att returerna ett skämt 
    if (*ptr == NULL) {
        printf("Det gick inte att allokera minnet\n");
        return -1;
    }
    memcpy(*ptr, buffer, 255); // kopiera skämtet till det allokerade minnesplatsen
    (*ptr)[255] = '\0'; // lägg en null terminator i arrayeb 

    return 0; // Success
}

int write_joke(char arr[255], int joke_length) { // skriver ett skämt till eeprom 
    if (joke_length > 255) {
        printf("Error: joke_length är större än buffertens storlek 255.\n");
        return 1;
    }

    memset(arr + joke_length, 0, 255 - joke_length); // Fylla resten av bufferten med nollor 

    return write_joke_pos(arr, 255, 0); // returnerar ett skämt som är skrivet till position noll 
}

int write_joke_pos(char arr[255], int joke_length, int pos) { // funktionen som ska skriva till en specfik position i eeprom 
    if (pos < 0) {
        printf("Fel: negativ position \n");
        return 1;
    }

    unsigned short address = pos * 255; // beräknar start adressen för positionen 
    if (address + joke_length > EEPROM_SIZE) {
        printf("Fel: adressen är utanför adressens intervall \n");
        return 1;
    }

    int remaining = joke_length; //antal byte kvar att skriva 
    int offset = 0; //offset i data 

    while (remaining > 0) {
        int page_offset = address % PAGE_SIZE; // beräkna sidans offset 
        int space_in_page = PAGE_SIZE - page_offset; // ledigt utrymme i sidan
        int write_length = (remaining < space_in_page) ? remaining : space_in_page;

        if (eeprom_write(address, (unsigned char*)&arr[offset], write_length) != 0) {
            printf("Fel: Kan inte skriva  till EEPROM på adressen 0x%04X\n", address);
            return 1;
        }

        address += write_length; // uppdaterar adressen 
        offset += write_length; //updaterar offseten
        remaining -= write_length; //minskar antal byte kvar att skriva 
    }

    return 0; // Success
}

int clear_eeprom(int ki_length) { //nollställer eeprom till 0 
    if (ki_length > EEPROM_SIZE) ki_length = EEPROM_SIZE;//begränsa storleken av eeprom 
    unsigned char *buffer = malloc(ki_length); //allokerar en buffert för nollor 
    if (!buffer) return 1; 
    memset(buffer, 0, ki_length); // fyller buffert med nollor 

    int result = 0; //resultatflagga
    int remaining = ki_length; 
    unsigned short address = 0;

    while (remaining > 0) {
        int page_offset = address % PAGE_SIZE;
        int space_in_page = PAGE_SIZE - page_offset;
        int write_length = (remaining < space_in_page) ? remaining : space_in_page;

        if (eeprom_write(address, &buffer[address], write_length) != 0) {
            result = 1;
            break;
        }

        address += write_length;
        remaining -= write_length;
    }

    free(buffer); // frigör minnet 
    return result; //retuernerar resultat
}

int fill_eeprom(int ki_length) { // fyller eeprom med 1or 
    if (ki_length > EEPROM_SIZE) ki_length = EEPROM_SIZE; // begränsa storleken av eeprom
    unsigned char *buffer = malloc(ki_length); // allokerar bufferten med ettor 
    if (!buffer) return 1;
    memset(buffer, 1, ki_length); // fylla buffert med ettor 

    int result = 0; 
    int remaining = ki_length;
    unsigned short address = 0;

    while (remaining > 0) {
        int page_offset = address % PAGE_SIZE;
        int space_in_page = PAGE_SIZE - page_offset;
        int write_length = (remaining < space_in_page) ? remaining : space_in_page;

        if (eeprom_write(address, &buffer[address], write_length) != 0) {
            result = 1;
            break;
        }

        address += write_length;
        remaining -= write_length;
    }

    free(buffer); // frigör minne
    return result;
}

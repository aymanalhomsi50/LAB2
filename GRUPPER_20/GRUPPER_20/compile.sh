#!/bin/bash
# compile.sh

# Skapa kompilationsmappen om den inte finns
mkdir -p bin

# Kompilera källkoden
gcc -o bin/dva271main main.c lib/DVA271_EEPROM.c lib/DVA271_GPIO.c lib/DVA_TEST.c -lpthread -lgpiod

# Kontrollera om kompileringen lyckades
if [ $? -eq 0 ]; then
    echo "Kompilering lyckades. Output finns i bin/dva271main"
else
    echo "Kompilering misslyckades."
    exit 1
fi

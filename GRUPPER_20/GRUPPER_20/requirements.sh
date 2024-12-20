#!/bin/bash
# requirements.sh

# Uppdatera paketlistan och installera nödvändiga paket
sudo pacman -Syu --noconfirm
sudo pacman -S --noconfirm gcc make gpiod i2c-tools

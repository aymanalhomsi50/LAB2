#!/bin/bash
# launchme.sh

# Starta programmet på CPU Core 3
taskset -c 3 ./bin/dva271main &
echo "Programmet startat på CPU Core 3"

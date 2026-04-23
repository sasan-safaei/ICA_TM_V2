#!/bin/bash
# SWD-uC-RESET (High)
echo 26 > /sys/class/gpio/export
# wait for sysfs to create the gpio folder
sleep 0.1
echo out > /sys/class/gpio/gpio26/direction
echo 0 > /sys/class/gpio/gpio26/value
# UART-Tx-Active (High)
echo 19 > /sys/class/gpio/export
# wait for sysfs to create the gpio folder
sleep 0.1
echo out > /sys/class/gpio/gpio19/direction
echo 1 > /sys/class/gpio/gpio19/value

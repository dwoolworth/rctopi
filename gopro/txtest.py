#!/usr/bin/env python

import RPi.GPIO as GPIO
import os, sys
import time

pin0 = 17
pin1 = 27
pin2 = 22

GPIO.setmode(GPIO.BCM)
GPIO.setwarnings(False)
GPIO.setup(pin0, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
GPIO.setup(pin1, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
GPIO.setup(pin2, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
GPIO.add_event_detect(pin0, GPIO.BOTH)
GPIO.add_event_detect(pin1, GPIO.BOTH)
GPIO.add_event_detect(pin2, GPIO.BOTH)

while True:
    if GPIO.event_detected(pin0) or GPIO.event_detected(pin1) or GPIO.event_detected(pin2):
        cmd = ((GPIO.input(pin0) << 0) | (GPIO.input(pin1) << 1) | (GPIO.input(pin2) << 2))
        print '%s' % (cmd)
        quit(0)
    time.sleep(0.01)


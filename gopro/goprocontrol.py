#!/usr/bin/python

import urllib.request
import RPi.GPIO as GPIO
import os, sys
import time
import argparse

# configure these - pull config from file somewhere...
gpv = 'h4'
wifipassword = 'master69'

# GoPro HERO4 WiFi settings - BlueSkyBots41 / master69

# GoPro Host
targetUrl       = "http://10.5.5.9/"

# GoPro Camera commands for HERO4
h4['On']            = "gp/gpControl/setting/10/0"
h4['Off']           = "gp/gpControl/setting/10/1"
h4['PhotoMode']     = "gp/gpControl/command/mode?p=1"
h4['VideoMode']     = "gp/gpControl/command/mode?p=0"
h4['BurstMode0']    = "gp/gpControl/command/mode?p=2"
h4['BurstMode1']    = "gp/gpControl/setting/70/0"
h4['Shutter']       = "gp/gpControl/command/shutter?p=1"
h4['Stop']          = "gp/gpControl/command/shutter?p=0"
h4['Tag']           = "gp/gpControl/command/storage/tag_moment"

# GoPro Camera Commands for HERO3
h3['On']            = "bacpac/PW?t=" + wifipassword + "&p=%01"
h3['Off']           = "bacpac/PW?t=" + wifipassword + "&p=%00"
h3['VideoMode']     = "camera/CM?t=" + wifipassword + "&p=%00"
h3['PhotoMode']     = "camera/CM?t=" + wifipassword + "&p=%01"
h3['BurstMode0']    = "camera/CM?t=" + wifipassword + "&p=%02"
h3['BurstMode1']    = "bacpac/SH?t=" + wifipassword + "&p=%01"
h3['Shutter']       = "bacpac/SH?t=" + wifipassword + "&p=%01"
h3['Stop']          = "bacpac/SH?t=" + wifipassword + "&p=%00"

camCmds['h4'] = h4
camCmds['h3'] = h3

# ATtiny85 input commands
t85CameraOff        = 0b000
t85PhotoMode        = 0b001
t85VideoMode        = 0b010
t85BurstAction      = 0b011
t85ToggleAction     = 0b100
t85ShutterAction    = 0b110
t85Aux1Action       = 0b111

def sendCmd(cmd):
    if cmd 
    data = urllib.request.urlretrieve(targetUrl + cmd)

def turnOffCamera():
    # check if camera is running first
    if cameraMode in (t85VideoMode, t85BurstAction) and actionRunning:
        # stop camera from running
        sendCmd(camCmds[gpv]['Stop'])
    # turn camera off
    sendCmd(camCmds[gpv]['Off']
    shutterToggle = False
    actionRunning = False
    cameraMode = t85CameraOff

def switchCameraMode(mode):
    # put camera in photo mode, if it's already in photo mode then reset
    # toggle to false.  If we're in video mode and we're running, stop first,
    # then toggle photo mode.  If we were in burst action, then stop bursting.
    if mode == t85PhotoMode:
        if cameraMode in (t85VideoMode, t85BurstAction) and actionRunning:
            # stop camera from running
            sendCmd(camCmds[gpv]['Stop'])
            actionRunning = False
        if cameraMode != t85PhotoMode:
            sendCmd(camCmds[gpv]['PhotoMode'])
    elif mode == t85VideoMode:
        if cameraMode == t85BurstAction and actionRunning:
            # stop camera from running
            sendCmd(camCmds[gpv]['Stop'])
            actionRunning = False
        if cameraMode != t85VideoMode:
            sendCmd(camCmds[gpv]['VideoMode'])
    elif mode == t85BurstAction:
        if cameraMode == t85VideoMode and actionRunning:
            # stop camera from running
            sendCmd(camCmds[gpv]['Stop'])
            actionRunning = False
        if cameraMode != t85BurstAction:
            sendCmd(camCmds[gpv]['BurstMode0'])
        sendCmd(camCmds[gpv]['BurstMode1'])
        actionRunning = True
    if cameraMode = t85CameraOff:
        # turn camera on
        sendCmd(camCmds[gpv]['On'])
    cameraMode = t85PhotoMode
    shutterToggle = False
    

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

cameraMode = t85CameraOff
shutterToggle = False
actionRunning = False

# Use GPIO.event_detected() because it will always recall an event that's occurred,
# and we can use a consistent while loop to do housekeeping, should it be necessary
while True:
    if GPIO.event_detected(pin0) or GPIO.event_detected(pin1) or GPIO.event_detected(pin2):
        cmd |= ((GPIO.input(pin0) << 0) | (GPIO.input(pin1) << 1) | (GPIO.input(pin2) << 2))
        if cmd & t85CameraOff and cameraMode != t85CameraOff:
            turnOffCamera()
        elif cmd & t85PhotoMode:
            switchCameraMode(t85PhotoMode)

        elif cmd & t85VideoMode:
            # put camera in video mode.  if it's already in video mode then reset
            # shutter toggle to false.  If we're in burst action, then stop bursting
            # and put in video mode.
            if cameraMode = t85CameraOff:
                # turn camera on
                sendCmd(camCmds[gpv]['On'])
            if cameraMode != t85VideoMode:
                sendCmd(camCmds[gpv]['VideoMode'])
            cameraMode = t85VideMode
            shutterToggle = False
        elif cmd & t85BurstAction:
            # put camera in burst mode and start, if it's already bursting, ignore, if
            # we were in video mode, stop video, change modes and start bursting.
        elif cmd & t85ToggleAction:
            # if we're in photo mode, if shuttertoggle is false, snap photo and
            # set shuttertoggle to true. if we're in video mode and shutter toggle is
            # false and videoRunning is false, start video, set shutter toggle to true
            # and video running to true.  If we are in burst action mode (shouldn't happen)
            # then stop burst, put in photo mode, take photo as above
        elif cmd & t85ShutterAction:
            # if we're in photo mode (shouldn't occur), do nothing, if we're in video
            # mode, then send a "tag_moment" command, set shutter toggle to true.
        elif cmd & t85Aux1Action:
            # ignore this action
            pass
        else:
            # ignore this action, although it potentially cannot occur
            pass

    # Do housekeeping here
    time.sleep(0.01)

















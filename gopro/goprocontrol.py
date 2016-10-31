#!/usr/bin/python

import urllib2
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
h4 = {}
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
h3 = {}
h3['On']            = "bacpac/PW?t=" + wifipassword + "&p=%01"
h3['Off']           = "bacpac/PW?t=" + wifipassword + "&p=%00"
h3['VideoMode']     = "camera/CM?t=" + wifipassword + "&p=%00"
h3['PhotoMode']     = "camera/CM?t=" + wifipassword + "&p=%01"
h3['BurstMode0']    = "camera/CM?t=" + wifipassword + "&p=%02"
h3['BurstMode1']    = "bacpac/SH?t=" + wifipassword + "&p=%01"
h3['Shutter']       = "bacpac/SH?t=" + wifipassword + "&p=%01"
h3['Stop']          = "bacpac/SH?t=" + wifipassword + "&p=%00"
h3['Tag']           = "bacpac/SH?t=" + wifipassword + "&p=%01"

camCmds = {}
camCmds['h4'] = h4
camCmds['h3'] = h3

# Mode states for ATtiny85 and internal status
modeCameraOff        = 0b000    # Off (0)
modePhotoMode        = 0b001    # Photo (1)
modeVideoMode        = 0b010    # Video (2)
modeBurstAction      = 0b011    # Burst (3)
modeToggleAction     = 0b100    # Toggle/Shutter (4)
modeShutterAction    = 0b101    # Toggle/Shutter (5)
modeTagMomentAction  = 0b110    # Tag Moment (6)
modeAux1Action       = 0b111    # Aux2 (7)

pin0 = 17
pin1 = 27
pin2 = 22
cameraMode = modeCameraOff
shutterToggle = False
actionRunning = False



def sendCmd(cmd):
    print '  => Sending Cmd: %s' % (targetUrl + cmd)
    sys.stdout.flush()
    #req = urllib2.Request(targetUrl + cmd)
    #response = urllib2.urlopen(req)
    #data = response.read()
    #print ' => %s' % (data)
    sys.stdout.flush()

def turnOffCamera():
    global cameraMode
    global shutterToggle
    global actionRunning
    # check if camera is running first
    if cameraMode in (modeVideoMode, modeBurstAction) and actionRunning:
        # stop camera from running
        sendCmd(camCmds[gpv]['Stop'])
    # turn camera off
    sendCmd(camCmds[gpv]['Off'])
    shutterToggle = False
    actionRunning = False
    cameraMode = modeCameraOff

def turnOnCamera():
    if cameraMode == modeCameraOff:
        sendCmd(camCmds[gpv]['On'])

def switchCameraMode(mode):
    global cameraMode
    global actionRunning
    global shutterToggle
    # make sure camera is on first
    turnOnCamera()
    # put camera in photo mode, if it's already in photo mode then reset
    # toggle to false.  If we're in video mode and we're running, stop first,
    # then toggle photo mode.  If we were in burst action, then stop bursting.
    if mode == modePhotoMode:
        if cameraMode in (modeVideoMode, modeBurstAction) and actionRunning:
            # stop camera from running
            sendCmd(camCmds[gpv]['Stop'])
            actionRunning = False
        if cameraMode != modePhotoMode:
            sendCmd(camCmds[gpv]['PhotoMode'])
        shutterToggle = False
    elif mode == modeVideoMode:
        if cameraMode == modeBurstAction and actionRunning:
            # stop camera from running
            sendCmd(camCmds[gpv]['Stop'])
            actionRunning = False
        if cameraMode != modeVideoMode:
            sendCmd(camCmds[gpv]['VideoMode'])
        shutterToggle = False
    elif mode == modeBurstAction:
        if cameraMode == modeVideoMode and actionRunning:
            # stop camera from running
            sendCmd(camCmds[gpv]['Stop'])
            actionRunning = False
        if cameraMode != modeBurstAction:
            sendCmd(camCmds[gpv]['BurstMode0'])
            sendCmd(camCmds[gpv]['BurstMode1'])
        actionRunning = True
        shutterToggle = False
    else:
        # bad command
        return
    cameraMode = mode

def toggleAndShutterAction():
    global cameraMode
    global actionRunning
    global shutterToggle
    # check mode and perform related action
    if cameraMode == modePhotoMode and not shutterToggle:
        # take a picture
        sendCmd(camCmds[gpv]['Shutter'])
        shutterToggle = True
    elif cameraMode == modeVideoMode and not shutterToggle:
        if not actionRunning:
            # start recording
            sendCmd(camCmds[gpv]['Shutter'])
            actionRunning = True
        else:
            # stop recording
            sendCmd(camCmds[gpv]['Stop'])
            actionRunning = False
        shutterToggle = True
    elif cameraMode == modeBurstAction:
        switchCameraMode(modePhotoMode)
        sendCmd(camCmds[gpv]['Shutter'])
        shutterToggle = True
    else:
        # no idea how we got here
        return

def tagVideoAction():
    global cameraMode
    global actionRunning
    global shutterToggle
    if cameraMode == modeVideoMode and not shutterToggle:
        if actionRunning:
            sendCmd(camCmds[gpv]['Tag'])
            shutterToggle = True

    
def initGPIO():
    global GPIO
    GPIO.setmode(GPIO.BCM)
    GPIO.setwarnings(False)
    GPIO.setup(pin0, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
    GPIO.setup(pin1, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
    GPIO.setup(pin2, GPIO.IN, pull_up_down=GPIO.PUD_DOWN)
    GPIO.add_event_detect(pin0, GPIO.BOTH)
    GPIO.add_event_detect(pin1, GPIO.BOTH)
    GPIO.add_event_detect(pin2, GPIO.BOTH)


###
# Main Entry Point
###

cmd = 0
newCmd = 0

print 'Initializing GPIO'
initGPIO()

# Use GPIO.event_detected() because it will always recall an event that's occurred,
# and we can use a consistent while loop to do housekeeping, should it be necessary
print 'Entering main loop'
sys.stdout.flush()
while True:
    if GPIO.event_detected(pin0) or GPIO.event_detected(pin1) or GPIO.event_detected(pin2):

        # Read pin status
        newCmd = ((GPIO.input(pin0) << 0) | (GPIO.input(pin1) << 1) | (GPIO.input(pin2) << 2))

        if newCmd == cmd:
            continue
        cmd = newCmd

        print 'Read command: %s' % (cmd)
        sys.stdout.flush()

        # Respond to commands
        if cmd == modeCameraOff and cameraMode != modeCameraOff:
            print 'Turning camera off'
            turnOffCamera()

        elif cmd == modePhotoMode:
            # put camera in photo mode
            print 'Switching to photo mode'
            switchCameraMode(modePhotoMode)

        elif cmd == modeVideoMode:
            # put camera in videomode
            print 'Switching to video mode'
            switchCameraMode(modeVideoMode)

        elif cmd == modeBurstAction:
            # put camera in burst mode and start
            print 'Switching to burst action'
            switchCameraMode(modeBurstAction)

        elif cmd == modeToggleAction or cmd == modeShutterAction:
            # snap photo, switch video on/off
            print 'Toggle photo/video shutter'
            toggleAndShutterAction()

        elif cmd == modeTagMomentAction:
            # if we're in photo mode (shouldn't occur), do nothing, if we're in video
            # mode, then send a "tag_moment" command, set shutter toggle to true.
            print 'Tag video moment'
            tagVideoAction()

        elif cmd == modeAux1Action:
            # ignore this action
            print 'Aux action, shouldn''t occur'
            pass
        else:
            # ignore this action, although it potentially cannot occur
            pass

    # Do housekeeping here
    sys.stdout.flush()
    time.sleep(0.01)



#!/bin/bash

{
    continue=True
    pm=20 # picture mode
    vm=36 # video mode
    bm=52 # burst mode
    ps=68 # picture shutter
    vs=84 # video shutter
    tm=100 # tag moment
    all=0
    total=0
    while [ "$continue" = True ]; do
        retval=`./txtest.py`
        i=$(((retval*16)+4))
        if [ $i -eq 4 ]; then
            i=0
        fi
        case $i in
        $pm)
            all=$(($all+$pm)) && pm=0
            total=$(($total+17))
            ;;
        $vm)
            all=$(($all+$vm)) && vm=0
            total=$(($total+17))
            ;;
        $bm)
            all=$(($all+$bm)) && bm=0
            total=$(($total+17))
            ;;
        $ps)
            all=$(($all+$ps)) && ps=0
            total=$(($total+17))
            ;;
        $vs)
            all=$(($all+$vs)) && vs=0
            total=$(($total+17))
            ;;
        $tm)
            all=$(($all+$tm)) && tm=0
            total=$(($total+17))
            ;;
        *) ;;
        esac
        if [ $total -gt 100 ]; then
            sleep 2
            continue=False
        fi
    done
} | whiptail --gauge "Cycle switches from Off to Photo, Video, Burst, Photo Shutter, Video Shutter, and Tag Moment..." 10 70 0 3>&1 1>&2 2>&3

whiptail --title "Success!" --msgbox

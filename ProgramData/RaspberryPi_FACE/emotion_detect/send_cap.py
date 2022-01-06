#!/usr/bin/env python3

import cv2
import emotion1
import time
import serial

camera = cv2.VideoCapture(0)

ser = serial.Serial('/dev/serial0', 9600)

send_data = 0

while True:
    c = emotion1.emotionVideo(camera)
    print(c)
    
    send_data = str(c)
    ser.write(bytes(send_data,'utf-8'))
    
    time.sleep(1)
ser.close()

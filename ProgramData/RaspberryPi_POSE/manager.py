#!/usr/bin/env python3

import org_pose_camera

import gspread
import json
import serial
import time

from oauth2client.service_account import ServiceAccountCredentials

#GoogleCloudPlatform Setup
print('GoogleCloudPlatform Setting...')
scope = ['https://spreadsheets.google.com/feeds','https://www.googleapis.com/auth/drive']
credentials = ServiceAccountCredentials.from_json_keyfile_name('/home/pi/coral/project-posenet/xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx.json', scope)
gc = gspread.authorize(credentials)
SPREADSHEET_KEY = 'xxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxxx'
worksheet = gc.open_by_key(SPREADSHEET_KEY).worksheet('xxxxxxx')

Ref_distance = 0
ERROR = 50

#Serial Setup(usb --> Spresense,rasp --> Raspberry Pi)
usbser = serial.Serial('/dev/ttyACM0', 9600, timeout = 0)
raspser = serial.Serial('/dev/serial0', 9600, timeout = 0)
usbser.flush()
raspser.flush()

time.sleep(1)
print('Start!')

#Spresense(Button1) --> RasPi(Pose)
#Pose Calibration
while True:
    temp_data = usbser.read()
    spresense_data = str(temp_data.decode())
    if spresense_data == 'A':
        Ref_distance = org_pose_camera.main()
        print('Pose_Calibration')
        usbser.write(b'B')
        break

#StartButton Check
while True:
    temp_data = usbser.read()
    spresense_data = str(temp_data.decode())
    if spresense_data == 'C':
        print('StartButton_Check')
        usbser.write(b'D')
        break

#Posture Maintenance
while True:
    #EmotionData
    receive_data = raspser.read()
    emotion_data = str(receive_data.decode())

    #PoseData
    pose_data = org_pose_camera.main()
    
    #SendData
    worksheet.update_cell(2,1, pose_data)
    worksheet.update_cell(2,2, emotion_data)

    if (Ref_distance - ERROR) > pose_data :
        print('Pose not good...')
        usbser.write(b'E')
    else:
        print('Pose OK!')
        usbser.write(b'F')
    
    temp_data = usbser.read()
    spresense_data = str(temp_data.decode())
    if spresense_data == 'G':
        print('PostureMaintenance_End')
        usbser.write(b'H')
        break

    time.sleep(25)

while True:
    temp_data = usbser.read()
    spresense_data = str(temp_data.decode())
    if spresense_data == 'I':
        final_emotion_data = str(worksheet.acell('J20').value)
        usbser.write(final_emotion_data)
        break
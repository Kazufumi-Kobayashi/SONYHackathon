import argparse
from functools import partial
import re
import time
import os
import math

import numpy as np
from PIL import Image
import cv2

from pose_engine import PoseEngine

send_pose_data = None
NOSEtoLFWRS = 0
NOSEtoRTWRS = 0

def main():
    parser = argparse.ArgumentParser(formatter_class=argparse.ArgumentDefaultsHelpFormatter)
    parser.add_argument('--mirror', help='flip video horizontally', action='store_true')
    parser.add_argument('--model', help='.tflite model path.', required=False)
    parser.add_argument('--res', help='Resolution', default='640x480',
                        choices=['480x360', '640x480', '1280x720'])
    parser.add_argument('--videosrc', help='Which video source to use (WebCam number or video file path)', default='0')
    # parser.add_argument('--h264', help='Use video/x-h264 input', action='store_true')
    args = parser.parse_args()

    default_model = 'models/mobilenet/posenet_mobilenet_v1_075_%d_%d_quant_decoder_edgetpu.tflite'
    if args.res == '480x360':
        src_size = (640, 480)
        appsink_size = (480, 360)
        model = args.model or default_model % (353, 481)
    elif args.res == '640x480':
        src_size = (640, 480)
        appsink_size = (640, 480)
        model = args.model or default_model % (481, 641)
    elif args.res == '1280x720':
        src_size = (1280, 720)
        appsink_size = (1280, 720)
        model = args.model or default_model % (721, 1281)

    print('Loading model: ', model)
    # engine = PoseEngine(model, mirror=args.mirror)
    engine = PoseEngine(model)


    last_time = time.monotonic()
    n = 0
    sum_fps = 0
    sum_process_time = 0
    sum_inference_time = 0

    width, height = src_size

    isVideoFile = False
    frameCount = 0
    maxFrames = 0

    NOSE_x = 0
    NOSE_y = 0
    RT_WRS_x = 0
    RT_WRS_y = 0
    LF_WRS_x = 0
    LF_WRS_y = 0

    # VideoCapture init
    videosrc = args.videosrc
    if videosrc.isdigit():
        videosrc = int(videosrc)
    else:
        isVideoFile = os.path.exists(videosrc)

    print("Start VideoCapture")
    cap = cv2.VideoCapture(videosrc)
    if cap.isOpened() == False:
        print('can\'t open video source \"%s\"' % str(videosrc))
        return;

    print("Open Video Source")
    cap.set(cv2.CAP_PROP_FPS, 60)
    cap.set(cv2.CAP_PROP_FRAME_WIDTH, width)
    cap.set(cv2.CAP_PROP_FRAME_HEIGHT, height)

    try:
        global send_pose_data
        global NOSEtoLFWRS
        global NOSEtoRTWRS
        ret, frame = cap.read()

        rgb = frame[:,:,::-1]

        image = Image.fromarray(rgb)
        outputs, inference_time = engine.DetectPosesInImage(image)
        #send_pose_data = outputs

        for pose in outputs:
            for label, keypoint in pose.keypoints.items():
                #print(label.name)
                if label.name == 'NOSE':
                    NOSE_x = keypoint.point[0]
                    NOSE_y = keypoint.point[1]
                if label.name == 'LEFT_WRIST':
                    LF_WRS_x = keypoint.point[0]
                    LF_WRS_y = keypoint.point[1]
                if label.name == 'RIGHT_WRIST':
                    RT_WRS_x = keypoint.point[0]
                    RT_WRS_y = keypoint.point[1]
                NOSEtoLFWRS = math.sqrt((LF_WRS_x - NOSE_x)**2 + (LF_WRS_y - NOSE_y)**2)
                NOSEtoRTWRS = math.sqrt((RT_WRS_x - NOSE_x)**2 + (RT_WRS_y - NOSE_y)**2)
         
        #print(outputs)
        print(NOSEtoRTWRS)
        #print(NOSEtoLFWRS)

    except Exception as ex:
        raise ex
    finally:
        cap.release()

    # return NOSEtoLFWRS
    return NOSEtoRTWRS

if __name__ == '__main__':
    main()
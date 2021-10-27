import cv2
import easygui
import time
import os
import sys
import importlib
import time
import shutil
import numpy as np
from libPOS import desktop
from opencvYOLO import yolov5
from tqdm import tqdm

# Product ID Declaration
labels = { "M01":["DL Full Cream Milk", 1.5], "S01":["Mentos Mint", 5], "T01":["Boh", 2.5],
           "C01":["Corn Flakes", 7.5], "M02":["DL Low Fat Milk", 1.5]}

# Initialization
idle_checkout = (5, 6)
dt = desktop("","")
flipFrame = (False,False) #(H, V)
lang = "EN"  #TW, EN

# YOLO Detection Backend
yolo = yolov5(yolo_type='yolov5',weights='/home/lry/POS/weights/last_3.onnx')

# GUI Setup
cv2.namedWindow("POS v1", cv2.WINDOW_FULLSCREEN)
cv2.setWindowProperty("POS v1", cv2.WND_PROP_FULLSCREEN, cv2.WINDOW_FULLSCREEN)

start_time = time.time()
dt.emptyBG = None
last_movetime = time.time()
YOLO_run = False
txtStatus = ""

# Object Detection
kernel = cv2.getStructuringElement(cv2.MORPH_ELLIPSE, (3, 3))
fgbg = cv2.createBackgroundSubtractorMOG2()
fourcc = cv2.VideoWriter_fourcc(*'XVID')



if __name__ == "__main__":

    INPUT = cv2.VideoCapture(0)
    width = int(INPUT.get(cv2.CAP_PROP_FRAME_WIDTH))
    height = int(INPUT.get(cv2.CAP_PROP_FRAME_HEIGHT))
    width = 480
    height = 360
    frameID = 0

    while True:
        hasFrame, frame = INPUT.read()

        bg = cv2.imread("/home/lry/POS/images/bg.jpg")

        resized = cv2.resize(frame, (480, 360))
        bg[95:95 + resized.shape[0], 25:25 + resized.shape[1]] = resized

        if not hasFrame:
            print("Process Complete")
            elapsed_time = time.time() - start_time
            print(f"{elapsed_time} seconds")
            break

        if dt.emptyBG is None:
            dt.emptyBG = np.zeros((640, 480))
            dt.emptyBG_time = time.time()
            print("Update BG")

        objects = dt.difference(dt.emptyBG, kernel, fgbg, frame, 20000)

        if objects>0:
            last_movetime = time.time()
            timeout_move = str(round(time.time()-last_movetime, 0))
            txtStatus = "Idle:" + timeout_move

        else:
            waiting = time.time() - last_movetime
            timeout_move = str(round(time.time()-last_movetime, 0))
            txtStatus = "Idle:" + timeout_move

            if waiting > idle_checkout[0] and waiting < idle_checkout[1]:
                txtStatus = "Calculate"
                YOLO_run = True

        imgDisplay = dt.display(frame.copy(), txtStatus)
        cv2.imshow("POS v1", imgDisplay)
        cv2.waitKey(1)

        if YOLO_run:
            YOLO_run = False
            #results = yolo.getObject(frame=frame, labelWant="", drawBox=True, bold=1, textsize=0.6, bcolor=(0,0,255), tcolor=(0,0,0))
            srcimg = frame.copy()
            dets = yolo.detect(frame)
            srcimg = yolo.postprocess(srcimg, dets)

            if len(yolo.classIdList) > 0:
                types = yolo.group(yolo.classIdList)
                print("Labels:", types)
                shoplist = []
                for items in types:
                    shoplist.append([items[0], labels[items[0]][0], labels[items[0]][1], len(items)])
            txtStatus = "checkout"
            #print(shoplist)

            imgDisplay = dt.display(srcimg.copy(), txtStatus, shoplist)
            cv2.imshow("POS v1", imgDisplay)
            cv2.waitKey(1)

            if (len(shoplist) > 0):
                print("Shop list:", shoplist)
                k = cv2.waitKey(0)
                if k == 0xFF & ord("p"):
                    yn = easygui.ynbox('Proceed to QR Code Payment?', 'Payment Method', ('Yes', 'No'))
                    if yn:
                        pay = cv2.imread('/home/lry/POS/images/Payment.jpg')
                        cv2.imshow('POS v1', pay)
                        cv2.waitKey(0)


            #cv2.imshow('Results', srcimg)
            #imgDisplay = dt.display(srcimg.copy(), txtStatus)
            #cv2.imshow("POS v1", imgDisplay)

        k = cv2.waitKey(1)
        if k == 0xFF & ord("q"):
            break
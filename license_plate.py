from ultralytics import YOLO
import cv2
import pytesseract
import re
import datetime
import os
# YOLO model
license_plate_model = YOLO("best.pt")

def logging_img(info, img, current_time):
    save_folder = './img/'
    if not os.path.isdir('img'):
        os.mkdir(save_folder)
    name = save_folder + current_time + '_' + info + '.jpg'
    cv2.imwrite(name, img)
    return 


# detect license plate from camera frame
def license_detect(camera=1):
    cap = cv2.VideoCapture(camera)
    ret, frame = cap.read()
    cap.release()
    if ret:
        license_plates = license_plate_model(frame)[0]

        if license_plates:
            current_time = datetime.datetime.today()
            current_time = current_time.strftime('%Y%m%d_%H_%M%S')
            logging_img('detect', frame, current_time)
            return (license_plates,frame)
        else:
            current_time = datetime.datetime.today()
            current_time = current_time.strftime('%Y%m%d_%H_%M%S')
            logging_img('no_detect', frame, current_time)
            print("no detect license")
            return (None,None)
    else:
        print("camara no read")
        return (None, None)
    
# obtain ocr from license_plate
def license_to_string(license_plates,frame):

    if not license_plates:
        print("license_plates is None")
        return None
    
    for license_plate in license_plates.boxes.data.tolist():
        x1, y1, x2, y2,score, class_id = license_plate
        
        # preprocess image
        license_plate_crop = frame[int(y1):int(y2),int(x1):int(x2),]
        license_plate_crop_gray = cv2.cvtColor(license_plate_crop,cv2.COLOR_BGR2GRAY)
        _,license_plate_crop_thresh=cv2.threshold(license_plate_crop_gray,64,255,cv2.THRESH_BINARY_INV)

        # OCR
        license_str = pytesseract.image_to_string(license_plate_crop_thresh)
        if license_str:
            print("ocr value :",license_str)
            print("ocr length :",len(license_str))
        if 5<len(license_str):
            # alpa and disit only
            re_str = re.sub('[^a-zA-Z0-9]','',license_str)
            print("re_str :",re_str)
            print("re str length :",len(re_str))
            current_time = datetime.datetime.today()
            current_time = current_time.strftime('%Y%m%d_%H_%M%S')
            logging_img('OCR', license_plate_crop_thresh, current_time)
            return re_str
        else:
            print("ocr_error and ocr length is less than 5")
    
    print("failed ocr")
    current_time = datetime.datetime.today()
    current_time = current_time.strftime('%Y%m%d_%H_%M%S')
    logging_img('no_OCR', license_plate_crop_thresh, current_time)
    return None


if __name__ == '__main__':
    license_plates,frame=license_detect(0)
    if license_plates:
        license_to_string(license_plates,frame)



    
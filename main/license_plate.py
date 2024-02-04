from ultralytics import YOLO
import cv2
import pytesseract
import re

# YOLO model
license_plate_model = YOLO("best.pt")

# detect license plate from camera frame
def license_detect(camera=1):
    cap = cv2.VideoCapture(camera)
    ret, frame = cap.read()
    cap.release()
    if ret:
        license_plates = license_plate_model(frame)[0]

        if license_plates:
            return (license_plates,frame)
        else:
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
        license_plate_crop = frame[int(y1):int(y2),int(x1):int(x2),]
        #cv2.imshow("crop",license_plate_crop)
        license_plate_crop_gray = cv2.cvtColor(license_plate_crop,cv2.COLOR_BGR2GRAY)
        #cv2.imshow("crop_gray",license_plate_crop_gray)
        _,license_plate_crop_thresh=cv2.threshold(license_plate_crop_gray,64,255,cv2.THRESH_BINARY_INV)
        #cv2.imshow("crop_thresh",license_plate_crop_thresh)
        license_str = pytesseract.image_to_string(license_plate_crop_thresh)
        if license_str:
            print("ocr value :",license_str)
            print("ocr length :",len(license_str))
        if 5<len(license_str):
            # alpa and disit only
            re_str = re.sub('[^a-zA-Z0-9]','',license_str)
            print("re_str :",re_str)
            print("re str length :",len(re_str))
            return re_str
        else:
            print("ocr_error and ocr length is less than 5")
    
    print("failed ocr")
    return None

if __name__ == '__main__':
    license_plates,frame=license_detect(0)
    if license_plates:
        license_to_string(license_plates,frame)

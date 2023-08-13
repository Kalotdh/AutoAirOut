import cv2
import RPi.GPIO as GPIO
import time

arduinoPin = 11

GPIO.setmode(GPIO.BOARD)
GPIO.setup(arduinoPin, GPIO.OUT)
GPIO.output(arduinoPin,GPIO.LOW)

def main():
    camera = cv2.VideoCapture(-1)
    camera.set(3,640)
    camera.set(4,480)
    
    face_xml = '/home/kalotdh/opencv/data/haarcascades/haarcascade_frontalface_default.xml'
    eye_xml = '/home/kalotdh/opencv/data/haarcascades/haarcascade_eye.xml'
    face_cascade = cv2.CascadeClassifier(face_xml)
    eye_cascade = cv2.CascadeClassifier(eye_xml)
    
    while( camera.isOpened() ):

        GPIO.output(arduinoPin, 0)
        faces_count = 0
        eyes_count = 0

        _, image = camera.read()
        gray = cv2.cvtColor(image, cv2.COLOR_BGR2GRAY)

        faces = face_cascade.detectMultiScale(gray,scaleFactor=1.1,minNeighbors=5,minSize=(20,20),flags=cv2.CASCADE_SCALE_IMAGE)
        faces_count = len(faces)
        print("faces count: " + str(faces_count))
        if faces_count:
            for (x,y,w,h) in faces:
                cv2.rectangle(image,(x,y),(x+w,y+h),(255,0,0),2)
                
                face_gray = gray[y:y+h, x:x+w]
                face_color = image[y:y+h, x:x+w]
                
                eyes = eye_cascade.detectMultiScale(face_gray,scaleFactor=1.1,minNeighbors=5)
                eyes_count = eyes_count + len(eyes)

                for (ex,ey,ew,eh) in eyes:
                    cv2.rectangle(face_color, (ex, ey), (ex+ew, ey+eh), (0,255,0), 2)
        
            print("eye count: " + str(eyes_count))                

            opened_eyes_count = eyes_count / faces_count

            if faces_count > 3 and opened_eyes_count <= 1:
                print("catch sleep: " + str(opened_eyes_count))
                GPIO.output(arduinoPin, 1)
                time.sleep(0.4)
                GPIO.output(arduinoPin, 0)
                time.sleep(1)
            else:
                GPIO.output(arduinoPin, 0)
        cv2.imshow('result', image)
        
        if cv2.waitKey(1) == ord('q'):
            break
    
    cv2.destroyAllWindows()
    GPIO.cleanup()

if __name__ == '__main__':
    main()

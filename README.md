IoT Automatic Fish Feeder

A web-based IoT system to monitor food levels and automate fish feeding. The project uses an ESP32 to control hardware components and a Python Flask backend hosted on Google Cloud Platform (GCP) to handle scheduling and remote control.

Overview

This system solves the problem of manual fish feeding by allowing users to:
    
    Monitor Feed Levels: Uses an ultrasonic sensor to calculate remaining feed percentage.
    Schedule Feeding: Users can set daily feeding times via a web interface.
    Remote Trigger: Manual "Feed Now" button available from the dashboard.
    Alerts: Local buzzer alarm on the device when feed is low.

Tech Stack
    
    Hardware: ESP32 (Dev Board), HC-SR04 Ultrasonic Sensor, Servo Motor (SG90/MG996R), Active Buzzer.
    Firmware: C/C++ (Arduino IDE).
    Backend: Python (Flask), Gunicorn.
    Frontend: HTML5, CSS3, JavaScript (jQuery, Chart.js).
    Infrastructure: Google Cloud Platform (Compute Engine VM).

Hardware Configuration

Bill of Materials

    ESP32 Development Board
    Standard Servo Motor
    HC-SR04 Ultrasonic Sensor
    Buzzer
    Jumper Wires & Power Supply (5V)

Pinout

    Mapping based on AqufeederIoTFINAL.ino:
        Component	ESP32 GPIO
        Ultrasonic Trig	5
        Ultrasonic Echo	18
        Servo Signal	13
        Buzzer	25

Setup & Deployment

1. Backend (Google Cloud VM)
   The backend runs on a GCP Compute Engine instance (Ubuntu/Debian).
   
         git clone https://github.com/Kuroihi/AquaFeederIoT
         cd fish-feeder-iot
         pip3 install -r requirements.txt
         python3 app.py
         gunicorn -w 4 -b 0.0.0.0:5000 app:app

Network Note: Ensure you have configured the GCP Firewall rules to allow TCP traffic on port 5000 (or whichever port you choose) so the ESP32 can reach the API.

2. Firmware (ESP32)
    Open AqufeederIoTFINAL.ino in Arduino IDE.
    Install required libraries: ArduinoJson, ESP32Servo, HTTPClient.
    Update the credentials and server endpoint:

       const char* ssid = "YOUR_WIFI_SSID";
       const char* password = "YOUR_WIFI_PASSWORD";
       const char* serverUrl = "http://YOUR_GCP_EXTERNAL_IP:5000/";
    Calibration: Adjust tankHeight (cm) in the code to match your physical container size. Upload to the board.

API Reference

Communication between ESP32 and Flask is done via HTTP polling.
    
    POST /update_sensor: Updates food percentage (JSON payload).
    GET /get_control: Checks for pending actions (Feed/Buzzer toggle).
    POST /set_schedule: Appends new schedule time.
    POST /feed_now: Triggers servo manually.

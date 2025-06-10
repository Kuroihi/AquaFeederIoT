from flask import Flask, render_template, jsonify, request
import threading
import time
from datetime import datetime
import pytz

app = Flask(__name__)

# Global variables for shared state
food_percentage = 0
buzzer_threshold = 5
buzzer_status = "off"
feed_status = "off"
schedules = []  # List of schedule times in format {"time": "HH:MM", "active": True}
sensor_data = []  # Store historical data for chart

# Lock for thread safety
lock = threading.Lock()

@app.route('/')
def index():
    # Jika ada ?partial=1, kembalikan JSON untuk auto-update
    if request.args.get('partial'):
        return jsonify(
            food_percentage=food_percentage,
            sensor_data=sensor_data
        )
    # Render full dashboard
    return render_template('index.html', 
                           food_percentage=food_percentage,
                           buzzer_threshold=buzzer_threshold,
                           buzzer_status=buzzer_status,
                           feed_status=feed_status,
                           schedules=schedules,
                           sensor_data=sensor_data)

# Route untuk ESP32 mengirim data sensor
@app.route('/update_sensor', methods=['POST'])
def update_sensor():
    global food_percentage, sensor_data, buzzer_status
    data = request.get_json()
    percentage = data.get('percentage', 0)
    
    with lock:
        food_percentage = percentage
        # Simpan dengan timestamp
        now = datetime.now(pytz.timezone('Asia/Jakarta'))
        timestamp = now.strftime("%H:%M")
        sensor_data.append({"time": timestamp, "value": percentage})
        if len(sensor_data) > 24:
            sensor_data.pop(0)
        # Auto-enable buzzer jika di bawah threshold
        if percentage < buzzer_threshold and buzzer_status == "off":
            buzzer_status = "on"
    
    return jsonify({"status": "success"})

# Route untuk ESP32 mengambil perintah kontrol
@app.route('/get_control', methods=['GET'])
def get_control():
    global buzzer_status, feed_status
    with lock:
        commands = {
            "buzzer": buzzer_status,
            "feed": feed_status
        }
        # Reset perintah feed setelah dikirim
        if feed_status == "on":
            feed_status = "off"
    return jsonify(commands)

# Route dari dashboard untuk set threshold
@app.route('/set_threshold', methods=['POST'])
def set_threshold():
    global buzzer_threshold
    data = request.get_json()
    threshold = data.get('threshold', 5)
    with lock:
        buzzer_threshold = threshold
    return jsonify({"status": "success"})

# Route dashboard untuk set jadwal
@app.route('/set_schedule', methods=['POST'])
def set_schedule():
    global schedules
    data = request.get_json()
    new_schedules = data.get('schedules', [])
    with lock:
        schedules = new_schedules
    return jsonify({"status": "success"})

# Route toggle buzzer
@app.route('/toggle_buzzer', methods=['POST'])
def toggle_buzzer():
    global buzzer_status
    with lock:
        buzzer_status = "on" if buzzer_status == "off" else "off"
    return jsonify({"status": "success", "buzzer_status": buzzer_status})

# Route manual feed
@app.route('/feed_now', methods=['POST'])
def feed_now():
    global feed_status
    with lock:
        feed_status = "on"
    return jsonify({"status": "success"})

# Thread background untuk cek jadwal
def check_schedule():
    global feed_status
    while True:
        now = datetime.now(pytz.timezone('Asia/Jakarta'))
        current_time = now.strftime("%H:%M")
        
        with lock:
            for schedule in schedules:
                if schedule['time'] == current_time and schedule['active']:
                    feed_status = "on"
                    break
        
        time.sleep(60)

# Jalankan thread
thread = threading.Thread(target=check_schedule)
thread.daemon = True
thread.start()

if __name__ == '__main__':
    app.run(debug=True)

import smbus2
import bme280
from flask import Flask, request
from flask import Response
from flask_cors import CORS
import json
import logging
import threading
import time
import subprocess
import re

sending=False

app = Flask(__name__)
app.config["DEBUG"] = True
CORS(app)

port = 1
address = 0x77
bus = smbus2.SMBus(port)

calibration_params = bme280.load_calibration_params(bus, address)

@app.route('/weather', methods=['GET'])
def get_weather():
    data = bme280.sample(bus, address, calibration_params)
    print(data)
    class Weather:
        def __init__(self, temperature, humidity, pressure):
            self.temperature = temperature
            self.humidity = humidity
            self.pressure = pressure
    w = Weather(data.temperature, data.humidity, data.pressure)
    jsonStr = json.dumps(w.__dict__)
    return Response(jsonStr, mimetype='application/json')

@app.route('/start', methods=['POST'])
def start_sending():
    global sending
    sending=True
    bashCommand = "hostname -I"
    process = subprocess.Popen(bashCommand.split(), stdout=subprocess.PIPE)
    output, error = process.communicate()
    hostname = re.findall("\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}", output.decode('utf-8'))
    x = threading.Thread(target=thread_function, args=(hostname[0],))
    x.start()
    return Response("", mimetype='application/json')

@app.route('/stop', methods=['POST'])
def stop_sending():
    global sending
    sending=False
    return Response("", mimetype='application/json')

def thread_function(name):
    while (sending):
        print("Thread %s: starting", name)
        time.sleep(2)
        print("Thread %s: finishing", name)

app.run(host="0.0.0.0", port=8080)

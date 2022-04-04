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
import requests

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
    
    server_name = json.loads(request.data)["hostname"]
    global sending
    sending=True
    bashCommand = "hostname -I"
    process = subprocess.Popen(bashCommand.split(), stdout=subprocess.PIPE)
    output, error = process.communicate()
    hostname = re.findall("\d{1,3}\.\d{1,3}\.\d{1,3}\.\d{1,3}", output.decode('utf-8'))
    x = threading.Thread(target=thread_function, args=(hostname[0], server_name))
    x.start()
    return Response("", mimetype='application/json')

@app.route('/stop', methods=['POST'])
def stop_sending():
    global sending
    sending=False
    return Response("", mimetype='application/json')

def thread_function(local_hostname, server_hostname):
    while (sending):
        print("Thread starting! Localhost: " + local_hostname + ". Server host: " + server_hostname)
        data = bme280.sample(bus, address, calibration_params)
        #print(data)
        w = {'temperature': data.temperature,'humidity': data.humidity, 'pressure': data.pressure, 'raspberryRoute': local_hostname}
        headers = {'Content-Type': 'application/json'}
        try:
            r = requests.post('http://'+ server_hostname + ':8080/measurement',json=w, headers=headers, timeout=10) # add authentication
            print(r.text)
        except:
            print("Error occurred while communicating with server")
        time.sleep(4)
        print("Thread finishing")

app.run(host="0.0.0.0", port=8080)

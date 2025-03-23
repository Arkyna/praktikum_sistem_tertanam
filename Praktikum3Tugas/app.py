from flask import Flask, render_template, jsonify
import firebase_admin
from firebase_admin import credentials, db
import os
import json

app = Flask(__name__)

# Mengambil Firebase Credential dari Environment Variable
firebase_config = json.loads(os.getenv("FIREBASE_CREDENTIALS"))

# Inisialisasi Firebase Admin SDK tanpa file JSON
cred = credentials.Certificate(firebase_config)
firebase_admin.initialize_app(cred, {
    'databaseURL': 'https://cloud-tugas3-default-rtdb.asia-southeast1.firebasedatabase.app/'
})

# Referensi ke data sensor
ref = db.reference('/EnvironmentData')

@app.route('/')
def index():
    """Menampilkan halaman utama"""
    snapshot = ref.get()
    if snapshot:
        time_value = snapshot.get('timestamp', 'unknown')
        light_condition = snapshot.get('condition', 'unknown')
        rain_value = snapshot.get('raining', False)
        ldr_value = snapshot.get('ldr_value', 0)
    else:
        time_value = 'unknown'
        light_condition = 'unknown'
        rain_value = False
        ldr_value = 0
    
    return render_template('index.html', time=time_value, condition=light_condition, rain=rain_value, ldr=ldr_value)

@app.route('/weather')
def get_weather():
    """API untuk mengambil data cuaca terbaru dalam format JSON"""
    snapshot = ref.get()
    if snapshot:
        time_value = snapshot.get('timestamp', 'unknown')
        light_condition = snapshot.get('condition', 'unknown')
        rain_value = snapshot.get('raining', False)
        ldr_value = snapshot.get('ldr_value', 0)
    else:
        time_value = 'unknown'
        light_condition = 'unknown'
        rain_value = False
        ldr_value = 0
    
    return jsonify({
        'time': time_value,
        'condition': light_condition,
        'rain': rain_value,
        'ldr_value': ldr_value
    })

if __name__ == '__main__':
    app.run(debug=True)

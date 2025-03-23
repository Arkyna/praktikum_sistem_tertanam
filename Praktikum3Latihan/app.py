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
    'databaseURL': 'https://cloud-pert3-daa38-default-rtdb.asia-southeast1.firebasedatabase.app/'
})

# Referensi ke data sensor
ref = db.reference('/WaterDistance')

@app.route('/')
def index():
    """Menampilkan halaman monitoring"""
    snapshot = ref.get()  # Ambil data terbaru
    distance = snapshot if snapshot else "Data belum tersedia"
    return render_template('index.html', distance=distance)

@app.route('/data')
def get_data():
    """API untuk mengambil data terbaru dalam format JSON"""
    snapshot = ref.get()
    data = {'distance': snapshot} if snapshot else {'distance': "Data belum tersedia"}
    return jsonify(data)

from flask import render_template, jsonify, make_response
import os 
from dotenv import load_dotenv
import firebase_admin
from firebase_admin import credentials, db
load_dotenv()
from io import BytesIO
from openpyxl import Workbook
class Route:
    def __init__(self, app):
        app.add_url_rule("/", "home", self.home)
        app.add_url_rule("/login", "login", self.login)
        app.add_url_rule("/register", "register", self.register)
        app.add_url_rule("/dashboard", "dashboard", self.dashboard)
        app.add_url_rule("/data/realtime", "realtime", self.realtime)
        app.add_url_rule("/data/history", "history", self.history)
        app.add_url_rule("/realtime", "data", self.fetchData)
        app.add_url_rule("/dump", "dump", self.dumpData)
        app.add_url_rule("/about", "about", self.about)
        app.add_url_rule("/export", "export", self.exportData)
        
        self.firebase_credentials = {
            "type": os.getenv("FIREBASE_TYPE"),
            "project_id": os.getenv("FIREBASE_PROJECT_ID"),
            "private_key_id": os.getenv("FIREBASE_PRIVATE_KEY_ID"),
            "private_key": os.getenv("FIREBASE_PRIVATE_KEY").replace("\\n", "\n"),
            "client_email": os.getenv("FIREBASE_CLIENT_EMAIL"),
            "client_id": os.getenv("FIREBASE_CLIENT_ID"),
            "auth_uri": os.getenv("FIREBASE_AUTH_URI"),
            "token_uri": os.getenv("FIREBASE_TOKEN_URI"),
            "auth_provider_x509_cert_url": os.getenv("FIREBASE_AUTH_PROVIDER_X509_CERT_URL"),
            "client_x509_cert_url": os.getenv("FIREBASE_CLIENT_X509_CERT_URL"),
        }
        
        self.cred = credentials.Certificate(self.firebase_credentials)
        firebase_admin.initialize_app(self.cred, {
            'databaseURL': os.getenv("FIREBASE_DATABASE_URL")
        })
        self.ref = db.reference('/')
        
    def home(self):
        return render_template('index.html')
    
    def login(self):
        return render_template('login.html')
    
    def register(self):
        return render_template('register.html')
    
    def dashboard(self):
        return render_template('dashboard.html');
    
    def realtime(self):
        return render_template('realtime.html')
    
    def history(self):
        return render_template('history.html')
    
    def about(self):
        return render_template('about.html')

    def fetchData(self):
        try:
        # Referensi ke root database atau node tertentu
            ref = db.reference('/data/realtime')  # Bisa disesuaikan dengan path data
            data = ref.get()  # Mendapatkan semua data
            return jsonify({"success": True, "data": data}), 200
        except Exception as e:
            return jsonify({"success": False, "error": str(e)}), 500
        
    def dumpData(self):
        try:
        # Referensi ke root database atau node tertentu
            ref = db.reference('/data/data')  # Bisa disesuaikan dengan path data
            data = ref.get()  # Mendapatkan semua data
            return jsonify({"success": True, "data": data}), 200
        except Exception as e:
            return jsonify({"success": False, "error": str(e)}), 500
        


    def exportData(self):
        try:
            # Referensi ke root database atau node tertentu
            ref = db.reference('/data/data')  # Bisa disesuaikan dengan path data
            data = ref.get()  # Mendapatkan semua data
            
            if not data:
                return jsonify({"success": False, "error": "No data found"}), 404

            # Membuat workbook baru
            wb = Workbook()
            ws = wb.active
            ws.title = "Data"

            # Menambahkan header
            headers = ["Key"] + list(next(iter(data.values())).keys())
            ws.append(headers)

            # Menambahkan data
            for key, value in data.items():
                row = [key] + list(value.values())
                ws.append(row)

            # Menyimpan workbook ke buffer
            output = BytesIO()
            wb.save(output)
            output.seek(0)

            # Membuat respons Flask dengan file Excel
            response = make_response(output.getvalue())
            response.headers['Content-Disposition'] = 'attachment; filename=data.xlsx'
            response.headers['Content-Type'] = 'application/vnd.openxmlformats-officedocument.spreadsheetml.sheet'
            
            return response
        except Exception as e:
            return jsonify({"success": False, "error": str(e)}), 500
    
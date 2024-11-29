import os,sys
sys.path.append(os.path.abspath(os.path.join("api")))
from flask import Flask
from route import Route

app = Flask(__name__)
route = Route(app)

if __name__ == "__main__":
    app.run( debug=True)
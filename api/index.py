from flask import Flask
from route import Route

app = Flask(__name__)
route = Route(app)

if __name__ == "__main__":
    app.run(host="0.0.0.0", debug=True)
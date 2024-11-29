from flask import render_template

class Route:
    def __init__(self, app):
        app.add_url_rule("/", "home", self.home)
        app.add_url_rule("/login", "login", self.login)
        
    def home(self):
        return render_template('index.html')
    
    def login(self):
        return render_template('login.html')
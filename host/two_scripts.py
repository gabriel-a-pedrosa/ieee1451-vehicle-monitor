import os
import subprocess

script_dir = os.path.dirname(os.path.abspath(__file__))

process1 = subprocess.Popen(["python", os.path.join(script_dir, "interface.py")])
process2 = subprocess.Popen(["python", os.path.join(script_dir, "gps.py")])

process1.wait()
process2.wait()
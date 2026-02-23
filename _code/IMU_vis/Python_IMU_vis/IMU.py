import serial
import os
import sys
import time
import numpy as np
from vedo import Mesh, Plotter, Text2D

# --- CONFIG ---
SERIAL_PORT = 'COM6' #//COM3 for notebook
BAUD_RATE = 115200
FILENAME = "GPS-Compass2.obj"
BASE_PATH = os.path.dirname(os.path.abspath(__file__))
MODEL_PATH = os.path.join(BASE_PATH, FILENAME)
UPDATE_INTERVAL = 0.05

# --- GLOBALS ---
euler_offset = [0.0, 0.0, 0.0]
current_euler = [0.0, 0.0, 0.0]
last_update = 0.0

def euler_to_matrix(roll, pitch, yaw):
    r, p, y = np.radians([roll, pitch, yaw])
    Rx = np.array([[1,0,0],[0,np.cos(r),-np.sin(r)],[0,np.sin(r),np.cos(r)]])
    Ry = np.array([[np.cos(p),0,np.sin(p)],[0,1,0],[-np.sin(p),0,np.cos(p)]])
    Rz = np.array([[np.cos(y),-np.sin(y),0],[np.sin(y),np.cos(y),0],[0,0,1]])
    return Rz @ Ry @ Rx

# --- SERIAL SETUP ---
try:
    ser = serial.Serial(SERIAL_PORT, BAUD_RATE, timeout=0.1)
    ser.setDTR(True)
    ser.setRTS(True)
except Exception as e:
    print(f"Serial Error: {e}")
    sys.exit()

# --- VEDO SETUP ---
plt = Plotter(axes=1, bg='white', size=(1000, 700), interactive=False)

if not os.path.exists(MODEL_PATH):
    print(f"ERROR: {FILENAME} not found at {MODEL_PATH}")
    sys.exit()

model = Mesh(MODEL_PATH).c('gold')
original_pts = model.vertices.copy()

try:
    model.add_shadow()
except Exception:
    pass

msg = Text2D("Initializing...", pos='top-left', c='black', font='VictorMono')

# --- KEYBOARD CALLBACK ---
def on_key(event):
    global euler_offset
    if event.keypress in ('z', 'Z'):
        euler_offset = current_euler.copy()
        print("\n>>> SENSOR ZEROED <<<\n")

# --- TIMER CALLBACK ---
def loop_func(event):
    global current_euler, last_update

    now = time.time()

    latest_line = None
    try:
        while ser.in_waiting > 0:
            line = ser.readline().decode('ascii', errors='ignore').strip()
            if "Euler:" in line:
                latest_line = line
    except Exception:
        pass

    if latest_line:
        print(f"RAW: {latest_line}")
        try:
            payload = latest_line.split(":")[1].split(",")
            current_euler = [float(p.strip()) for p in payload]
        except Exception:
            pass

    if now - last_update < UPDATE_INTERVAL:
        return
    last_update = now

    roll  = current_euler[0] - euler_offset[0]
    pitch = current_euler[1] - euler_offset[1]
    yaw   = current_euler[2] - euler_offset[2]

    R = euler_to_matrix(pitch, roll, yaw)
    model.vertices = (R @ original_pts.T).T

    new_info = (f" LIVE DATA (Press 'z' to Zero)\n"
                f" Roll:  {roll:+.2f}°\n"
                f" Pitch: {pitch:+.2f}°\n"
                f" Yaw:   {yaw:+.2f}°")
    msg.text(new_info)
    plt.render()

# --- REGISTER CALLBACKS & LAUNCH ---
plt.add_callback('key press', on_key)
plt.add_callback('timer', loop_func)
plt.timer_callback('start', dt=10)
plt.show(model, msg, interactive=True)
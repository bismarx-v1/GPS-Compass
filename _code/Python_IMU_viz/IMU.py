import serial
import os
import sys
from vedo import Mesh, Plotter, Text2D

# --- CONFIG ---
SERIAL_PORT = 'COM3'
BAUD_RATE = 115200
FILENAME = "GPS-Compass.obj"
BASE_PATH = os.path.dirname(os.path.abspath(__file__))
MODEL_PATH = os.path.join(BASE_PATH, FILENAME)

# --- GLOBAL VARIABLES FOR TARE ---
q_offset = [1.0, 0.0, 0.0, 0.0]
current_raw = [1.0, 0.0, 0.0, 0.0]

def quat_inverse(q):
    return [q[0], -q[1], -q[2], -q[3]]

def quat_mult(q1, q2):
    w1, x1, y1, z1 = q1
    w2, x2, y2, z2 = q2
    return [
        w1*w2 - x1*x2 - y1*y2 - z1*z2,
        w1*x2 + x1*w2 + y1*z2 - z1*y2,
        w1*y2 - x1*z2 + y1*w2 + z1*x2,
        w1*z2 + x1*y2 - y1*x2 + z1*w2
    ]

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
try:
    model.add_shadow()
except Exception:
    pass

msg = Text2D("Initializing...", pos='top-left', c='black', font='VictorMono')

# --- KEYBOARD CALLBACK ---
def on_key(event):
    global q_offset
    if event.key == 'z':
        q_offset = quat_inverse(current_raw)
        print("\n>>> SENSOR ZEROED <<<\n")

# --- TIMER CALLBACK ---
def loop_func(event):
    global current_raw
    if ser.in_waiting > 0:
        try:
            line = ser.readline().decode('ascii', errors='ignore').strip()
            if line:
                print(f"RAW: {line}")
            if "Quaternion:" in line:
                payload = line.split(":")[1].split(",")
                current_raw = [float(p.strip()) for p in payload]
                q_display = quat_mult(q_offset, current_raw)
                model.orientation(q_display)
                w, x, y, z = q_display
                new_info = f" LIVE DATA (Press 'z' to Zero) \n W: {w:+.4f}\n X: {x:+.4f}\n Y: {y:+.4f}\n Z: {z:+.4f}"
                msg.text(new_info)
                plt.render()
        except Exception:
            pass

# --- REGISTER CALLBACKS & LAUNCH ---
plt.add_callback('key press', on_key)
plt.add_callback('timer', loop_func)
plt.timer_callback('start', dt=10)
plt.show(model, msg, interactive=True)
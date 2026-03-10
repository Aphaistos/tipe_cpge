import socket
import pyaudio
import threading
import time
import struct

CHUNK = 1024
FORMAT = pyaudio.paInt16
CHANNELS = 1
RATE = 44100

running = True

def handle_exit():
    global running
    input("Press ENTER to stop...\n")
    running = False

# Setup audio input
p = pyaudio.PyAudio()
stream = p.open(format=FORMAT,
                channels=CHANNELS,
                rate=RATE,
                input=True,
                frames_per_buffer=CHUNK)

# Connect to server
client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
client_socket.connect(("localhost", 5000))

print("Connected. Sending audio...")

threading.Thread(target=handle_exit, daemon=True).start()

while running:
    data = stream.read(CHUNK)
    timestamp = time.time()
    packed_time = struct.pack("!d", timestamp)  # 8 bytes
    
    client_socket.sendall(packed_time + data)    
print("Stopping client...")
client_socket.close()
stream.stop_stream()
stream.close()
p.terminate()
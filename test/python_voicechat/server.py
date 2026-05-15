import socket
import pyaudio
import struct
import threading
import time

CHUNK = 1024
HEADER_SIZE = 8
FORMAT = pyaudio.paInt16
CHANNELS = 1
RATE = 44100

running = True

def handle_exit():
    global running
    input("Press ENTER to stop...\n")
    running = False

def recv_exact(conn, size):
    data = b''
    while len(data) < size:
        packet = conn.recv(size - len(data))
        if not packet:
            return None
        data += packet
    return data

# Setup audio output
p = pyaudio.PyAudio()
stream = p.open(format=FORMAT,
                channels=CHANNELS,
                rate=RATE,
                output=True,
                frames_per_buffer=CHUNK)

# Setup socket server
server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
server_socket.bind(("0.0.0.0", 5000))
server_socket.listen(1)

print("Waiting for connection...")
conn, addr = server_socket.accept()
print(f"Connected from {addr}")

threading.Thread(target=handle_exit, daemon=True).start()

while running:
    packet = conn.recv(HEADER_SIZE + CHUNK)
    if not packet:
        break

    timestamp = struct.unpack("!d", packet[:HEADER_SIZE])[0]
    audio_data = packet[HEADER_SIZE:]

    latency = (time.time() - timestamp) * 1000  # ms
    print(f"Latency: {latency:.2f} ms")

    stream.write(audio_data)
    
    
print("Stopping server...")
conn.close()
server_socket.close()
stream.stop_stream()
stream.close()
p.terminate()
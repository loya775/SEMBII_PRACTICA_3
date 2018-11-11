import socket
from math import*
import numpy as np
import time

UDP_IP = "192.168.0.202"

UDP_PORT = 50000

socket_client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
socket_client.connect((UDP_IP, UDP_PORT))

buffer_size = 250;

Fs=1000
f=1
sample = 1000;
song_data=[0]*sample
for n in range(sample):
    song_data[n]= 2046*sin(2*pi*f*n/Fs) + 2048

song_data = np.cast[np.uint16](song_data)

sample_delay = float(buffer_size)/Fs;

send_start = 0;
send_end = buffer_size;
cont_song = 0;

while 1:

    socket_client.sendto(song_data[send_start:send_end], (UDP_IP, UDP_PORT))

    #print(song_data[send_start:send_end])
    if cont_song < (len(song_data)/buffer_size)-1:
    	cont_song = cont_song+1
    else:
    	cont_song = 0

    send_start = cont_song*buffer_size
    send_end = (cont_song+1)*buffer_size-1

    time.sleep((sample_delay)) 
    
socket_client.close()

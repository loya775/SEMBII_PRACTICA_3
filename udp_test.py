import socket
from math import*
import numpy as np
import time
import matplotlib.pyplot as plt
import scipy.io.wavfile as waves

UDP_IP = "192.168.0.105"

UDP_PORT = 55000

socket_client = socket.socket(socket.AF_INET, socket.SOCK_DGRAM)
socket_client.connect((UDP_IP, UDP_PORT))

buffer_size = 250;

archivo = 'muestra01_ElAguacateIntro.wav'
muestreo, sonido = waves.read(archivo)

Fs=muestreo
print(len(sonido))
f=5
sample = buffer_size*4;
print(muestreo)
song_data=[0]*sample

#for n in range(sample):
#    song_data[n]= 2046*sonido(n/Fs) + 2048

#song_data = np.cast[np.uint16](song_data)

#sample_delay = float(buffer_size)/Fs;

#send_start = 0;
#send_end = buffer_size;
#cont_song = 0;

#socket_client.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1);
#while 1:

#    socket_client.sendto(song_data[send_start:send_end], (UDP_IP, UDP_PORT))

    #print(song_data[send_start:send_end])
#    if cont_song < (len(song_data)/buffer_size)-1:
#    	cont_song = cont_song+1
#    else:
#    	cont_song = 0

 #   send_start = cont_song*buffer_size
#    send_end = (cont_song+1)*buffer_size-1

#    time.sleep((sample_delay)) 
    
#socket_client.close()

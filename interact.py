import serial
import pyaudio
import numpy as np
import wave
import scipy.signal as signal
import warnings

def serial_init(speed):
    dev = serial.Serial(
        port='/dev/ttyUSB0', 
        baudrate=speed,
        parity=serial.PARITY_NONE,
        stopbits=serial.STOPBITS_ONE,
        bytesize=serial.EIGHTBITS,
        timeout=0.1
    )
    return dev

def serial_recv(dev):
    # Для простоты макс. кол-во символов для чтения - 255. Время ожидания - 0.1
    # decode необходим для конвертирования набора полученных байтов в строку
    string = dev.read(255).decode()
    return string

def serial_send(dev, cmd):
    dev.write(bytearray([cmd]))

if __name__ == '__main__':
	dev = serial_init(115200)
    
	inputStr = ""
	while True:
		inputStr = input()

		for ch in inputStr:
			if ch == 'a': serial_send(dev, 0)
			if ch == 'd': serial_send(dev, 1)
			if ch == 'w': serial_send(dev, 2)
			if ch == 's': serial_send(dev, 3)
			if ch == 'j': serial_send(dev, 4)
			if ch == 'l': serial_send(dev, 5)
			if ch == 'i': serial_send(dev, 6)
			if ch == 'k': serial_send(dev, 7)
			if ch == ' ': serial_send(dev, 8)

		if inputStr ==  "play": serial_send(dev, 9)
		if inputStr == "pause": serial_send(dev, 10)
		if inputStr == "reset": serial_send(dev, 11)
		if inputStr ==  "quit": break
		

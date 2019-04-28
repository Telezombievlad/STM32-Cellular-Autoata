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

		if inputStr == "l":
			serial_send(dev, 0)
		if inputStr == "r":
			serial_send(dev, 1)
		if inputStr == "u":
			serial_send(dev, 2)
		if inputStr == "d":
			serial_send(dev, 3)
		if inputStr == "quit":
			break
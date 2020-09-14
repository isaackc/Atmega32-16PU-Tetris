main = tetris.c
output = main.elf
includes = ./include/avr.c ./include/led_control.c

all: 
	avr-gcc -mmcu=atmega32 -o $(output) $(main) $(includes)
debug:
	gcc tetris.c
upload:
	avrdude -p m32 -P /dev/ttyACM0 -c avrisp -b 19200 -U flash:w:$(output)
jtag:
	avrdude -p m32 -P /dev/ttyACM0 -c avrisp -b 19200 -U lfuse:w:0xff:m -U hfuse:w:0xd9:m 
clean:
	rm $(output)

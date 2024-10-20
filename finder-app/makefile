CROSS_COMPILE=aarch64-none-linux-gnu-
all: writer.o
	aarch64-none-linux-gnu-gcc -o writer writer.c
default: writer.o
	gcc -I CROSS_COMPILE=aarch64-none-linux-gnu- -o writer writer.c
clean:
	rm writer writer.o

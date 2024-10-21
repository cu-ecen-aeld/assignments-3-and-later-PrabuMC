CROSS_COMPILE=aarch64-none-linux-gnu-
all: writer.o
	gcc -o writer writer.c -I
default: writer.o
	gcc -I CROSS_COMPILE=aarch64-none-linux-gnu- -o writer writer.c -I
clean:
	rm writer writer.o

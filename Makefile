default: writer.o
	gcc -o writer writer.c -I
clean:
	rm writer writer.o

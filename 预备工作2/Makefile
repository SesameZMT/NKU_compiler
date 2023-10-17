CC=arm-linux-gnueabihf-gcc
CFLAGS=-static
TARGET=main

run: $(TARGET)
	./$^

$(TARGET): main.S
	$(CC) $^ -o $@ $(CFLAGS)

clean:
	rm -f $(TARGET)


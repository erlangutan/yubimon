all: yubimon

yubimon: clean
	gcc -L/usr/local/lib -I /usr/local/include/libusb-1.0 -lusb-1.0 -o bin/yubimon yubimon.c

clean:
	rm -f bin/yubimon

install:
	bin/install

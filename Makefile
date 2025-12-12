.PHONE: build

build:
	rsync main.c pi:/tmp
	ssh pi -t "cd /tmp && gcc -o main main.c -lgpiod && ./main"


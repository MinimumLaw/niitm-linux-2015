all: app

app: app.c

run: app
	sudo mknod kbuf c 248 0
	sudo ./app
	echo with ags
	sudo ./app "let be args"
	rm -f kbuf


clean:
	rm -f app kbuf

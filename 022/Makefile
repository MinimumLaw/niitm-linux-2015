CC=gcc
LDFLAGS+=-lrt -pthread

all: rwqueue monitor

rwqueue: rwqueue.c
	${CC} -o rwqueue rwqueue.c ${CFLAGS} ${LDFLAGS}

monitor: monitor.c
	${CC} -o monitor monitor.c ${CFLAGS} ${LDFLAGS}

run: all
	echo Starting main rwqueue task
	./rwqueue 5 5 50&
	echo [OK] - Show stats every 3 secs.
	while true; do ./monitor; sleep 3; done;
#	killall rwqueue

clean:
	rm -f monitor rwqueue
CFLAGS ?= -I/usr/local/include/netana
LDFLAGS ?= -L/usr/local/lib

all:
	${CC} ${CFLAGS} ${LDFLAGS} netanalyzer_demo.c -o netanalyzer_demo -lnetana
clean:
	rm -f *.o netanalyzer_demo

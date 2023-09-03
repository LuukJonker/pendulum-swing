CC = g++

define CFLAGS
-O3 \
-Wpedantic \
-Wall \
-Wextra \
-Wformat=2 \
-Wsign-conversion \
-Wswitch-enum \
-Wno-switch-default \
-Wunused \
-Wfloat-equal \
-Wconversion \
-Wpointer-arith \
-Wtype-limits \
-Wcast-qual \
-Wenum-compare \
-Wsizeof-pointer-memaccess \
-Wstrict-prototypes
endef

define GLUTFLAGS
-lGL \
-lGLU \
-lglut
endef

LDFLAGS = -lm

RELEASEFLAGS = -O3
DEBUGFLAGS = -g -O0 -DDEBUG -fsanitize=address

PROG = swing

all: $(PROG)

swing: swing.o
	$(CC) -o $@ $^ $(LDFLAGS) $(GLUTFLAGS) $(RELEASEFLAGS)

debug: swing.o
	$(CC) -o $@ $^ $(LDFLAGS) $(GLUTFLAGS) $(DEBUGFLAGS)

clean:
	rm -f *.o $(PROG)

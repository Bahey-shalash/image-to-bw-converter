# ===== File: Makefile =====
CC      := gcc
CFLAGS  := -O3 -fPIC -Wall -Wextra -I.
LDFLAGS :=

# Sources
LIB_SRC := bw_converter.c
CLI_SRC := image_bw_converter_altium.c
LIB_OBJ := $(LIB_SRC:.c=.o)
CLI_OBJ := $(CLI_SRC:.c=.o)

# Targets
all: image_bw_converter libbwconvert.so

image_bw_converter: $(CLI_OBJ) libbwconvert.so
	$(CC) $(CFLAGS) -o $@ $(CLI_OBJ) -L. -lbwconvert

libbwconvert.so: $(LIB_OBJ)
	$(CC) -shared -o $@ $(LIB_OBJ)

%.o: %.c
	$(CC) $(CFLAGS) -c $<

clean:
	rm -f *.o image_bw_converter libbwconvert.so
ROOT=../../build
include $(ROOT)/rules.mk
.PHONY: clean

INCS += -I../include

OBJS = mm-test.o \

all: credit-card-app

credit-card-app: $(OBJS)
	 	 $(CC) $(CFLAGS) $(INCS) -o $@ $(OBJS)

clean:
	rm -rf *.o *~ core credit-card-app 



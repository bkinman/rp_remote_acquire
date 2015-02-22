
OBJS = main.o options.o scope.o transfer.o
SRCS = $(subst .o,.c, $(OBJS)))

TARGET=rp_remote_acquire


CFLAGS=-g -std=gnu99 -Wall -Werror -fPIC
CFLAGS += -DVERSION=$(VERSION) -DREVISION=$(REVISION)

LIBS=-lm -lpthread

CC=$(CROSS_COMPILE)gcc

INSTALL_DIR ?= .

all: $(TARGET)

%.o: %.c
	$(CC) -c $(CFLAGS) $< -o $@

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(CFLAGS) $(LIBS)

clean:
	rm -f $(TARGET) *.o

install:
	mkdir -p $(INSTALL_DIR)/bin
	cp $(TARGET) $(INSTALL_DIR)/bin
	mkdir -p $(INSTALL_DIR)/src/utils/$(TARGET)
	-rm -f $(TARGET) *.o
	cp -r * $(INSTALL_DIR)/src/utils/$(TARGET)/
	-rm `find $(INSTALL_DIR)/src/tools/$(TARGET)/ -iname .svn` -rf

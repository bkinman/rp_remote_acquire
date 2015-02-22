CC=$(CROSS_COMPILE)gcc
RM=rm

OBJECTS=main.o

EXT_SRC_DIR=../../../src
EXT_OBJS=$(EXT_SRC_DIR)/options.o $(EXT_SRC_DIR)/scope.o $(EXT_SRC_DIR)/transfer.o
EXT_INC=-I$(EXT_SRC_DIR)

INCLUDE=$(EXT_INC)

#CFLAGS=$(CFLAGS) -Wall -Werror -g
CFLAGS+= -Wall -Werror -g -fPIC $(INCLUDE)
LDFLAGS=-shared

OUT_DIR=../
C_OUT_NAME=$(OUT_DIR)controller.so

all: make_c_app
clean: clean_c_app

$(EXT_OBJS):
	$(MAKE) -C $(EXT_SRC_DIR)

make_c_app: $(EXT_OBJS) $(OBJECTS)
	$(CC) -o $(C_OUT_NAME) $(OBJECTS) $(EXT_OBJS) $(CFLAGS) $(LDFLAGS)

clean_c_app:
	$(RM) -f $(C_OUT_NAME) $(OBJECTS)

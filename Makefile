SRCPATH = ./src
OBJPATH = ./obj
OUTPATH = ./out
TARGET = touchcursor
# LIBS = -lm
CC = gcc
CFLAGS = -g -Wall
INSTALLPATH?=/usr/local/bin

.PHONY: default all clean

default: $(OUTPATH)/$(TARGET)
all: default

# All c files, excluding test.c
SOURCES = $(filter-out $(SRCPATH)/test.c, $(wildcard $(SRCPATH)/*.c))
# All h files
HEADERS = $(wildcard $(SRCPATH)/*.h)
# Replace c files with obj/filename.o from sources
OBJECTS = $(patsubst $(SRCPATH)/%.c, $(OBJPATH)/%.o, $(SOURCES))

.PRECIOUS: $(TARGET) $(OBJECTS)

# Each .o file depends on its .c file and .h file (we include all headers)
$(OBJPATH)/%.o: $(SRCPATH)/%.c $(HEADERS)
	@mkdir -p $(OBJPATH)
	$(CC) $(CFLAGS) -c $< -o $@

# The target depends on all the .o files
$(OUTPATH)/$(TARGET): $(OBJECTS)
	@mkdir -p $(OUTPATH)
	$(CC) $(OBJECTS) -Wall -o $@

clean:
	-rm -f obj/*.o
	-rm -f $(OUTPATH)/$(TARGET)

install:
	@echo "# Copying application to /usr/local/bin/"
	@echo "# This action requires sudo."
	sudo cp $(OUTPATH)/$(TARGET) $(INSTALLPATH)
	@echo ""
	@echo "# Changing the group on the application to the same group as the devices in /dev/input/."
	@echo "# This allows the application to read from the input without needing sudo."
	@echo "# This action requires sudo."
	sudo chgrp --reference=$$(find /dev/input/by-id/ | grep kbd | head -n 1) $(INSTALLPATH)/$(TARGET)
	@echo ""
	@echo "# Setting the groupid sticky bit on the application."
	@echo "# This allows the application to run as the group we just assigned."
	@echo "# This action requires sudo."
	sudo chmod g+s $(INSTALLPATH)/$(TARGET)

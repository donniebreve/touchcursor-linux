SRCPATH = ./src
OBJPATH = ./obj
OUTPATH = ./out
TARGET = touchcursor
# LIBS = -lm
CC = gcc
CFLAGS = -g -Wall
INSTALLPATH?=/usr/bin
# Configuration variables
SERVICEPATH = /etc/systemd/system
SERVICEFILE = touchcursor@.service
SERVICE := touchcursor@$(USER).service
CONFIGPATH = /etc/touchcursor
CONFIGFILE = touchcursor.conf
INPUTFILE = $(shell find /dev/input/event* | head -n 1)
INPUTGROUP = $(shell stat -c '%G' $(INPUTFILE))

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
	@echo "# Copying application to $(INSTALLPATH)"
	@echo "# This action requires sudo."
	sudo cp $(OUTPATH)/$(TARGET) $(INSTALLPATH)
	@echo ""

	@echo "# Copying default configuration file to $(CONFIGPATH)/$(CONFIGFILE)"
	@echo "# This action requires sudo."
	sudo mkdir -p $(CONFIGPATH)
	sudo cp -n $(CONFIGFILE) $(CONFIGPATH)
	@echo ""
	
	@echo "# Copying service file to $(SERVICEPATH)"
	@echo "# This action requires sudo."
	sed -i 's/Group=input/Group=$(INPUTGROUP)/' $(SERVICEFILE)
	sudo cp $(SERVICEFILE) $(SERVICEPATH)
	@echo ""
	
	@echo "# Enabling and starting the service"
	@echo "# This action requires sudo."
	sudo systemctl daemon-reload
	sudo systemctl enable $(SERVICE)
	sudo systemctl start $(SERVICE)

uninstall:
	@echo "# Stopping and disabling the service"
	@echo "# This action requires sudo."
	-sudo systemctl daemon-reload
	-sudo systemctl stop $(SERVICE)
	-sudo systemctl disable $(SERVICE)
	@echo ""

	@echo "# Removing service file from $(SERVICEPATH)"
	@echo "# This action requires sudo."
	-sudo rm $(SERVICEPATH)/$(SERVICEFILE)
	@echo ""

	@echo "# Removing application from $(INSTALLPATH)"
	@echo "# This action requires sudo."
	-sudo rm $(INSTALLPATH)/touchcursor
	@echo ""

	@echo "# Removing configuration file $(CONFIGPATH)/$(CONFIGFILE)"
	@echo "# This action requires sudo."
	-sudo rm $(CONFIGPATH)/$(CONFIGFILE)
	-sudo rm -r $(CONFIGPATH)

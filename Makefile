SRCPATH = ./src
OBJPATH = ./obj
OUTPATH = ./out
TARGET = touchcursor
# LIBS = -lm
CC = gcc
CFLAGS = -g -Wall
INSTALLPATH?=/usr/bin
# Configuration variables
SERVICEPATH = $(HOME)/.config/systemd/user
SERVICEFILE = touchcursor.service
SERVICE := touchcursor.service
CONFIGPATH = $(HOME)/.config/touchcursor
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
	@echo "# Stopping the service"
	-systemctl --user stop $(SERVICE)
	@echo ""
	
	@echo "# Copying application to $(INSTALLPATH)"
	@echo "# This action requires sudo."
	sudo cp $(OUTPATH)/$(TARGET) $(INSTALLPATH)
	sudo chmod u+s $(INSTALLPATH)/$(TARGET)
	@echo ""

	@echo "# Copying default configuration file to $(CONFIGPATH)/$(CONFIGFILE)"
	mkdir -p $(CONFIGPATH)
	cp -n $(CONFIGFILE) $(CONFIGPATH)
	@echo ""
	
	@echo "# Copying service file to $(SERVICEPATH)"
	mkdir -p $(SERVICEPATH)
	cp -f $(SERVICEFILE) $(SERVICEPATH)
	@echo ""
	
	@echo "# Enabling and starting the service"
	systemctl --user daemon-reload
	systemctl --user enable $(SERVICE)
	systemctl --user start $(SERVICE)

uninstall:
	@echo "# Stopping and disabling the service"
	-systemctl --user stop $(SERVICE)
	-systemctl --user disable $(SERVICE)
	@echo ""

	@echo "# Removing configuration file $(CONFIGPATH)/$(CONFIGFILE)"
	-rm $(CONFIGPATH)/$(CONFIGFILE)
	-rm -r $(CONFIGPATH)
	@echo ""

	@echo "# Removing service file from $(SERVICEPATH)"
	-rm $(SERVICEPATH)/$(SERVICEFILE)
	-rm -d $(SERVICEPATH)
	@echo ""

	@echo "# Removing application from $(INSTALLPATH)"
	@echo "# This action requires sudo."
	-sudo rm $(INSTALLPATH)/$(TARGET)
	@echo ""

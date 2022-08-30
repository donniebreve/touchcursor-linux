SRCPATH = ./src
OBJPATH = ./obj
OUTPATH = ./out
TARGET = touchcursor
# LIBS = -lm
CC = gcc
CFLAGS = -g -Wall
# INSTALLPATH ?= $(HOME)/.local/bin
INSTALLPATH ?= /usr/bin
# OLDINSTALLPATH ?= /usr/bin/touchcursor
# Configuration variables
SERVICE-PATH = $(HOME)/.config/systemd/user
SERVICE-FILE = touchcursor.service
SERVICE-PATH-WATCHER := touchcursor.path
CONFIGPATH = $(HOME)/.config/touchcursor
CONFIGFILE = touchcursor.conf

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
	-systemctl --user stop $(SERVICE-PATH-WATCHER)
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
	
	@echo "# Copying service files to $(SERVICE-PATH)"
	mkdir -p $(SERVICE-PATH)
	cp -f $(SERVICE-FILE) $(SERVICE-PATH)
	cp -f $(SERVICE-PATH-WATCHER) $(SERVICE-PATH)
	@echo ""
	
	@echo "# Enabling and starting the service"
	systemctl --user daemon-reload
	systemctl --user enable --now $(SERVICE-PATH-WATCHER)

uninstall:
	@echo "# Stopping and disabling the service"
	-systemctl --user disable --now $(SERVICE-PATH-WATCHER)
	-systemctl --user daemon-reload
	@echo ""

	@echo "# Removing configuration file $(CONFIGPATH)/$(CONFIGFILE)"
	-rm $(CONFIGPATH)/$(CONFIGFILE)
	-rm -r $(CONFIGPATH)
	@echo ""

	@echo "# Removing service files from $(SERVICE-PATH)"
	-rm $(SERVICE-PATH)/$(SERVICE-FILE)
	-rm $(SERVICE-PATH)/$(SERVICE-PATH-WATCHER)
	-rm -d $(SERVICE-PATH)
	@echo ""

	@echo "# Removing application from $(INSTALLPATH)"
	@echo "# This action requires sudo."
	-sudo rm $(INSTALLPATH)/$(TARGET)
	@echo ""

SRCPATH = ./src
OBJPATH = ./obj
OUTPATH = ./out
TARGET = touchcursor
# LIBS = -lm
CC = gcc
CFLAGS = -g -Wall
INSTALLPATH?=/$(HOME)/.local/bin
# Configuration variables
SERVICEPATH = $(HOME)/.config/systemd/user
SERVICEFILE = touchcursor.service
SERVICE := touchcursor.path
SERVICEPATHFILE = touchcursor.path
SERVICETARGETPATH = default.taget.wants
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
	-systemctl --user stop $(SERVICE)
	@echo ""
	
	@echo "# Copying application to $(INSTALLPATH)"
	cp $(OUTPATH)/$(TARGET) $(INSTALLPATH)
	chmod u+s $(INSTALLPATH)/$(TARGET)
	@echo ""

	@echo "# Copying default configuration file to $(CONFIGPATH)/$(CONFIGFILE)"
	mkdir -p $(CONFIGPATH)
	cp -n $(CONFIGFILE) $(CONFIGPATH)
	@echo ""
	
	@echo "# Copying service files to $(SERVICEPATH)"
	mkdir -p $(SERVICEPATH)
	cp -f $(SERVICEFILE) $(SERVICEPATH)
	cp -f $(SERVICEPATHFILE) $(SERVICEPATH)
	@echo ""
	
	@echo "# Enabling and starting the service"
	systemctl --user daemon-reload
	systemctl --user preset $(SERVICE)
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
	-rm $(SERVICEPATH)/$(SERVICEPATHFILE)
	-rm $(SERVICEPATH)/$(SERVICETARGETPATH)/$(SERVICEPATHFILE)
	-rm -d $(SERVICEPATH)
	@echo ""

	@echo "# Removing application from $(INSTALLPATH)"
	-rm $(INSTALLPATH)/$(TARGET)
	@echo ""

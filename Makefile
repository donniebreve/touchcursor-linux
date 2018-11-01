SRCPATH = ./src
OBJPATH = ./obj
OUTPATH = ./out
TARGET = touchcursor
# LIBS = -lm
CC = gcc
CFLAGS = -g -Wall
INSTALLPATH?=/usr/bin

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
	@echo "# Copying default configuration file to ~/.config/touchcursor/touchcursor.config"
	mkdir -p ~/.config/touchcursor/
	cp -n touchcursor.config ~/.config/touchcursor/touchcursor.config
	@echo ""
	@echo "# Copying application to /usr/bin/"
	@echo "# This action requires sudo."
	sudo cp $(OUTPATH)/$(TARGET) $(INSTALLPATH)
	@echo ""
	@echo "# Copying service file to /etc/systemd/system/"
	@echo "# This action requires sudo."
	sudo cp touchcursor@.service /etc/systemd/system/
	sudo sed -i s/Group=/Group=$$(ls -al /dev/input | grep '^c' |  head -n 1 | awk '{print $$4}')/ /etc/systemd/system/touchcursor@.service
	@echo ""
	@echo "# Enabling and starting the service"
	@echo "# This action requires sudo."
	sudo systemctl daemon-reload
	sudo systemctl enable touchcursor@$(USER).service
	sudo systemctl start touchcursor@$(USER).service
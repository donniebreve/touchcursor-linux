# Overridable variables
INSTALLPATH ?= /usr/bin
SERVICEPATH ?= $(HOME)/.config/systemd/user
CONFIGPATH ?= $(HOME)/.config/touchcursor

# Application variables
service = touchcursor.service
config = touchcursor.conf

# Build variables
src_path = ./src
obj_path = ./obj
out_path = ./out
binary = touchcursor
# LIBS = -lm
cc = gcc
cflags = -g -Wall
# All .h files
headers = $(wildcard $(src_path)/*.h)
# All .c files, excluding test.c
sources = $(filter-out $(src_path)/test.c, $(wildcard $(src_path)/*.c))
# Replace .c files with obj/filename.o from sources
objects = $(patsubst $(src_path)/%.c, $(obj_path)/%.o, $(sources))

# The binary depends on all .o files
# This is the main target of the make file
$(out_path)/$(binary): $(objects)
	@mkdir -p $(out_path)
	$(cc) $(objects) -Wall -o $@

# Each .o file depends on its .c file and .h file (we include all headers)
$(obj_path)/%.o: $(src_path)/%.c $(headers)
	@mkdir -p $(obj_path)
	$(cc) $(cflags) -c $< -o $@

clean:
	-rm -f obj/*.o
	-rm -f $(out_path)/$(binary)

debug: $(out_path)/$(binary)
	@echo "# Stopping the service"
	-systemctl --user stop $(service)
	@echo ""

	@echo "# Chown root $(out_path)/$(binary)"
	sudo chown root $(out_path)/$(binary)
	@echo "# Chmod u+s $(out_path)/$(binary)"
	sudo chmod u+s $(out_path)/$(binary)
	@echo "# Run $(out_path)/$(binary)"
	$(out_path)/$(binary)

install:
	@echo "# Stopping and disabling the service"
	-systemctl --user disable --now $(service)
	@echo ""

	@echo "# Copying application to $(INSTALLPATH)"
	@echo "# This action requires sudo."
	sudo cp $(out_path)/$(binary) $(INSTALLPATH)
	sudo chmod u+s $(INSTALLPATH)/$(binary)
	@echo ""

	@echo "# Copying service file to $(SERVICEPATH)"
	mkdir -p $(SERVICEPATH)
	cp -f $(service) $(SERVICEPATH)
	@echo ""

	@echo "# Copying default configuration file to $(CONFIGPATH)/$(config)"
	mkdir -p $(CONFIGPATH)
	cp -n $(config) $(CONFIGPATH)
	@echo ""

	@echo "# Enabling and starting the service"
	systemctl --user daemon-reload
	systemctl --user enable --now $(service)

uninstall:
	@echo "# The configuration file will not be removed:"
	@echo "# $(CONFIGPATH)/$(config)"
	@echo ""
	@echo "# To uninstall everything (including the configuration file)"
	@echo "# run 'make uninstall-full'"
	@echo ""

	@echo "# Stopping and disabling the service"
	-systemctl --user disable --now $(service)
	systemctl --user daemon-reload
	@echo ""

	@echo "# Removing service file from $(SERVICEPATH)"
	-rm $(SERVICEPATH)/$(service)
	-rm -d $(SERVICEPATH)
	@echo ""

	@echo "# Removing application from $(INSTALLPATH)"
	@echo "# This action requires sudo."
	-sudo rm $(INSTALLPATH)/$(binary)
	@echo ""

uninstall-full:
	@echo "# Stopping and disabling the service"
	-systemctl --user disable --now $(service)
	systemctl --user daemon-reload
	@echo ""

	@echo "# Removing service file from $(SERVICEPATH)"
	-rm $(SERVICEPATH)/$(service)
	-rm -d $(SERVICEPATH)
	@echo ""

	@echo "# Removing application from $(INSTALLPATH)"
	@echo "# This action requires sudo."
	-sudo rm $(INSTALLPATH)/$(binary)
	@echo ""

	@echo "# Removing configuration file $(CONFIGPATH)/$(config)"
	-rm $(CONFIGPATH)/$(config)
	-rm -d $(CONFIGPATH)
	@echo ""

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
cflags = -Wall
ldflags = -pthread
# All .h files
headers = $(wildcard $(src_path)/*.h)
# All .c files, excluding test.c
sources = $(filter-out $(src_path)/test.c, $(wildcard $(src_path)/*.c))
# Replace .c files with obj/filename.o from sources
objects = $(patsubst $(src_path)/%.c, $(obj_path)/%.o, $(sources))

# This is the main target of the make file
$(out_path)/$(binary): $(objects)
	@mkdir --parents $(out_path)
	$(cc) $(objects) $(ldflags) -o $@

# Each .o file depends on its .c file and .h file (we include all headers)
$(obj_path)/%.o: $(src_path)/%.c $(headers)
	@mkdir --parents $(obj_path)
	$(cc) $(cflags) -c $< -o $@

# This is the test binary target of the make file
test_binary = touchcursor_test
test_sources = $(filter-out $(src_path)/emit.c $(src_path)/main.c, $(wildcard $(src_path)/*.c))
test_objects = $(patsubst $(src_path)/%.c, $(obj_path)/%.o, $(test_sources))
$(out_path)/$(test_binary): $(test_objects)
	@mkdir --parents $(out_path)
	$(cc) $(test_objects) $(ldflags) -o $@

check: $(out_path)/$(test_binary)
	$(out_path)/$(test_binary)

clean:
	-rm --force obj/*.o
	-rm --force $(out_path)/*

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
	sudo cp --force $(out_path)/$(binary) $(INSTALLPATH)
	sudo chmod u+s $(INSTALLPATH)/$(binary)
	@echo ""

	@echo "# Copying service file to $(SERVICEPATH)"
	mkdir --parents $(SERVICEPATH)
	cp --force $(service) $(SERVICEPATH)
	@echo ""
	
	@echo "# Copying default configuration file to $(CONFIGPATH)/$(config)"
	mkdir --parents $(CONFIGPATH)
	-cp --no-clobber $(config) $(CONFIGPATH)
	@echo ""

	@echo "# Do you want to add all currently connected keyboards to your configuration? (y/N)"
	@read -r answer; \
	if [ "$$answer" = "y" ] || [ "$$answer" = "Y" ]; then \
		echo ""; \
		echo "Updating configuration..."; \
		bash ./scripts/update_conf_with_all_keyboards.sh $(CONFIGPATH)/$(config); \
	fi
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
	-rm --dir $(SERVICEPATH)
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
	-rm --dir $(SERVICEPATH)
	@echo ""

	@echo "# Removing application from $(INSTALLPATH)"
	@echo "# This action requires sudo."
	-sudo rm $(INSTALLPATH)/$(binary)
	@echo ""

	@echo "# Removing configuration file $(CONFIGPATH)/$(config)"
	-rm $(CONFIGPATH)/$(config)
	-rm --dir $(CONFIGPATH)
	@echo ""

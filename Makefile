BIN_NAME    := convert

GOT_GTK4 = $(shell pkg-config --exists gtk4 && echo 1 || echo 0)
BIN_FILES    := src/main.c src/file_io.c src/image_edit.c src/flood_fill.c
ifeq ($(GOT_GTK4), 0)
	BIN_FILES += src/gui_caller_fallback.c
else
	BIN_FILES += src/gui.c src/gui_caller.c
endif
HEADERS      := $(wildcard src/*.h)

TEST_SCRIPT := test/run_tests.py
Q ?= @

DEBUG   := -O0 -g -fsanitize=address -fsanitize=undefined
OPT     := -O3

PKGCONFIG = $(shell which pkg-config)
CFLAGS  += -Isrc -Wall -Wno-unused-command-line-argument -Wno-deprecated-declarations -Wextra -pedantic 
ifeq ($(GOT_GTK4), 1)
	CFLAGS += $(shell $(PKGCONFIG) --cflags gtk4)
endif
LDFLAGS +=
LIBS = 
ifeq ($(GOT_GTK4), 1)
	LIBS += $(shell $(PKGCONFIG) --libs gtk4)
endif

.PHONY: all check clean

GUI_INFO = 
ifeq ($(GOT_GTK4), 0)
		GUI_INFO += GTK4 not found, built without GUI
else
		GUI_INFO += GTK4 found, built with GUI
endif

all: bin/$(BIN_NAME)_opt bin/$(BIN_NAME)
	@echo "$(GUI_INFO)"

bin/$(BIN_NAME)_opt: $(patsubst src/%.c, build/%.opt.o, $(BIN_FILES))
	$(Q)mkdir -p $(@D)
	@echo "===> LD $@"
	$(Q)$(CC) -o $@ $(CFLAGS) $(OPT) $+ $(LDFLAGS) $(LIBS)

bin/$(BIN_NAME): $(patsubst src/%.c, build/%.debug.o, $(BIN_FILES))
	$(Q)mkdir -p $(@D)
	@echo "===> LD $@"
	$(Q)$(CC) -o $@ $(CFLAGS) $(DEBUG) $+ $(LDFLAGS) $(LIBS)

bin/$(TESTER_NAME): $(patsubst src/%.c, build/%.debug.o, $(TESTER_FILES))
	$(Q)mkdir -p $(@D)
	@echo "===> LD $@"
	$(Q)$(CC) -o $@ $(CFLAGS) $(DEBUG) $+ $(LDFLAGS) $(LIBS)

build/%.opt.o: src/%.c $(HEADERS)
	$(Q)mkdir -p $(@D)
	@echo "===> CC $@"
	$(Q)$(CC) -o $@ -c $(CFLAGS) $(OPT) $(LIBS) $< 

build/%.debug.o: src/%.c $(HEADERS)
	$(Q)mkdir -p $(@D)
	@echo "===> CC $@"
	$(Q)$(CC) -o $@ -c $(CFLAGS) $(DEBUG) $(LIBS) $<

check: all
	@echo "===> CHECK"
	$(Q)$(TEST_SCRIPT)

clean:
	@echo "===> CLEAN"
	$(Q)rm -rf bin build

extract_imgs.tgz:
	@make -C test/data > /dev/null

all: extract_imgs.tgz

extract_refs.tgz:
	@make -C test/ref_output > /dev/null

all: extract_refs.tgz


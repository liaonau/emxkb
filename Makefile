PKGS      = x11

INCS     := $(shell pkg-config --cflags $(PKGS)) -I./
CFLAGS   := -std=gnu18 -ggdb -W -Wall -Wextra $(INCS) $(CFLAGS)

LIBS     := $(shell pkg-config --libs $(PKGS))
LDFLAGS  := $(LIBS) $(LDFLAGS) -Wl,--export-dynamic

SRCS  = $(wildcard *.c)
HEADS = $(wildcard *.h)
OBJS  = $(foreach obj,$(SRCS:.c=.o),$(obj))

emxkb: $(OBJS)
	@echo $(CC) -o $@ $(OBJS)
	@$(CC) -o $@ $(OBJS) $(LDFLAGS)

$(OBJS): $(HEADS)

.c.o:
	@echo $(CC) -c $< -o $@
	@$(CC) -c $(CFLAGS) $(CPPFLAGS) $< -o $@

clean:
	rm -f emxkb $(OBJS)

all: emxkb


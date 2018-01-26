EXE      = rmolder
OBJDIR   = obj
CFLAGS   = -Wall -O3 -MMD -MP
CSOURCES := $(wildcard *.c)
OBJFILES := $(CSOURCES:%.c=$(OBJDIR)/%.o)
DEPFILES := $(OBJFILES:.o=.d)

.phony: all clean

all: $(EXE)

clean:
	/bin/rm -rf $(OBJDIR) $(EXE)

$(OBJDIR):
	mkdir -p $@

$(EXE): $(OBJDIR) $(OBJFILES)
	$(CC) $(CFLAGS) $(OBJFILES) -o $@

$(OBJDIR)/%.o : %.c
	$(CC) -c $(CFLAGS) -o $@ $<

-include $(DEPFILES)

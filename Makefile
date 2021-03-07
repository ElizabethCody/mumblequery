CC=gcc
CFLAGS=-Wall -Werror -pedantic -g -O2
LDFLAGS=
BUILDDIR=build
OBJS = main.o hostport.o mumblequery.o
OBJFILES = $(addprefix $(BUILDDIR)/,$(OBJS))

all: mumblequery

mumblequery: $(BUILDDIR)/mumblequery
	cp $(BUILDDIR)/mumblequery mumblequery

$(BUILDDIR)/mumblequery: $(OBJFILES)
	$(CC) $(OBJFILES) -o $(BUILDDIR)/mumblequery $(LDFLAGS)

# this hack breaks make's up-to-date check
# TODO: fix this
$(BUILDDIR)/%.o: %.c $(BUILDDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILDDIR):
	mkdir "$(BUILDDIR)"

clean:
	$(RM) mumblequery && $(RM) -r "$(BUILDDIR)"

#
# Triangle Solver Makefile 06/30/2015
#
#

#uncomment these lines for compile for x86
ARCH=x86
CROSS_COMPILE=

CC = $(CROSS_COMPILE)gcc
LD = $(CROSS_COMPILE)ld
STRIP = $(CROSS_COMPILE)strip
DIRS = $(. application)
PREFIX = /usr/local

INCLUDE = -v -I./ 

DEBUG = -Wall -ggdb3

CFLAGS = $(DEBUG) -fPIC $(LIB) $(INCLUDE)  -g

#Add some flags for more or less debug info...
#CFLAGS += -DMAINDEBUG		#triangle.c
CFLAGS += -DDEBUGLEVEL=0	#0 is less verbose, increase this number for more verbose

PROG = triangle
SETUP_SCRIPT = triangle.sh


# add .o's as you add files...
OBJS = 	./triangle.o 

###
## Shouldn't need to change below here
#

#all: $(PROG) $(SETUP_SCRIPT)
all: $(PROG)

.c .o:
	$(CC) $(CFLAGS) -c $< -o $@

$(PROG): $(OBJS)
	$(CC) $(CFLAGS) -o $(PROG) $(OBJS) $(LIBS)

strip:
	-$(STRIP) --strip-unneeded $(PROG)

install : $(DESTDIR)/$(PREFIX)/bin
	chmod +x $(SETUP_SCRIPT)
	cp -a $(SETUP_SCRIPT) $(DESTDIR)/$(PREFIX)/bin/
	cp -a $(PROG) $(DESTDIR)/$(PREFIX)/bin/
	
	
    
$(DESTDIR)/$(PREFIX)/bin:
	mkdir -p $@

uninstall:
	-rm -f $(BINDIR)/$(PROG) 
	-rm -f $(BINDIR)/$(SETUP_SCRIPT)

static: $(OBJS)
	$(CC) $(CFLAGS) -static -o $(PROG) $(OBJS) $(LIBS)

distclean: clean
	-rm -f $(PROG) cscope.*

clean: 
	-rm -f ./core* ./*.o ./*/*.o ./version.h ./*~ ./*/*~

cscope:
	-find . -name "*.[chS]" >cscope.files
	-cscope -b -R

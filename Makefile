SYSCONF_LINK = g++
CCFLAGS      =
LDFLAGS      =
LIBS         = -lm
DEBUG        = -g 

DESTDIR = ./
TARGET  = main

OBJECTS := $(patsubst %.cc,%.o,$(wildcard *.cc))

all: $(DESTDIR)$(TARGET)

$(DESTDIR)$(TARGET): $(OBJECTS)
	$(SYSCONF_LINK) -Wall $(LDFLAGS) -o $(DESTDIR)$(TARGET) $(OBJECTS) $(LIBS) $(DEBUG)

$(OBJECTS): %.o: %.cc
	$(SYSCONF_LINK) -Wall $(CCFLAGS) -c $(CFLAGS) $< -o $@ $(DEBUG)

clean:
	-rm -f $(OBJECTS)
	-rm -f $(TARGET)
	-rm -f *.tga

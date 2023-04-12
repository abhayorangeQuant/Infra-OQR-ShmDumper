# Makefile to build Market Data
CC=/usr/bin/g++
CFLAGS=-g -std=c++0x
LDFLAGS = -L/usr/local/lib -L/usr/lib -L/usr/lib64/mysql/ -lpthread -lrt -lmysqlclient

ifeq ($(OS), REDHAT)
CC=g++
CFLAGS=-O3 -std=c++11
LDFLAGS = -L/usr/local/lib -L/usr/lib -L/usr/lib64/mysql/ -lpthread -lrt -lmysqlclient
endif

BIN=../bin

INCLUDE= -I./ -I/usr/include/mysql/ -I../Common -I../LiveCode -I../BarData -I../Database

SOURCES= ../BarData/Bar.cpp \
	../BarData/MakeBar.cpp \
	../Database/DBConnector.cpp \
	../LiveCode/ShmSPMCConsumer.cpp \
	../Common/SettingFileReader.cpp \
	Strategy.cpp \
	StrategyBase.cpp \
	Ticker.cpp \
	main.cpp


OBJECTS=$(SOURCES:.cpp=.o)

EXECUTABLE=$(BIN)/ShmRecorder

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	$(CC) $(OBJECTS) $(LDFLAGS) -o $@

.cpp.o:
	$(CC) -c $(CFLAGS) $(INCLUDE) $< -o $@


# --- remove binary and executable files
clean:
	rm $(OBJECTS) $(EXECUTABLE)

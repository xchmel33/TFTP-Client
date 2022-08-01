CC=g++
CFLAGS= -g -Wall

TARGET = mytftpclient

all: clean $(TARGET)

$(TARGET): $(TARGET).cpp
	$(CC) $(CFLAGS) -o $(TARGET) $(TARGET).cpp

clean:
	$(RM) $(TARGET)

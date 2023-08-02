CC = gcc
CFLAGS = -std=c11 -Wall -Werror
LDFLAGS =
OBJFILES = connect.o constants.o execute.o parse.o rake-c.o
TARGET = rake-c

all: $(TARGET)

$(TARGET): $(OBJFILES)
	$(CC) $(CFLAGS) -o $(TARGET) $(OBJFILES) $(LDFLAGS)

clean:
	rm -f $(OBJFILES) $(TARGET) *~
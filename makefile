CC = gcc
CFLAGS = -Wall

TARGET = classroomSynchronization
RM = rm

all: $(TARGET)

$(TARGET): $(TARGET).c ; $(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c -lpthread

clean: ; $(RM) $(TARGET)  

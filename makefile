CC = gcc
CFLAGS = -Wall

TARGET = classroomSynchronizationMutex
TARGET2 = classroomSynchronizationSemaphore
RM = rm

all: $(TARGET) $(TARGET2)

$(TARGET): $(TARGET).c ; $(CC) $(CFLAGS) -o $(TARGET) $(TARGET).c -lpthread

$(TARGET2): $(TARGET2).c ; $(CC) $(CFLAGS) -o $(TARGET2) $(TARGET2).c -lpthread -lrt

clean: ; $(RM) $(TARGET); $(RM) $(TARGET2)   

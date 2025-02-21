TARGET = custom-touchpad-gesture
C_SOURCES =  custom-touchpad-gesture.c

CC = gcc 

.PHONY: custom-touchpad-gesture clean
begin:
	$(CC) -g $(C_SOURCES) -o $(TARGET)

clean:
	-rm -Rf $(TARGET)

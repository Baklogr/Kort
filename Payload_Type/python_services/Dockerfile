FROM itsafeaturemythic/mythic_base_image:latest

RUN apt-get update && apt-get install -y \
    make \
    gcc-mingw-w64-x86-64 \
    binutils-mingw-w64-x86-64 \
    git \
    && rm -rf /var/lib/apt/lists/*

RUN pip3 install mythic_container==0.5.25 donut-shellcode

WORKDIR /Mythic/
CMD ["python3", "main.py"]
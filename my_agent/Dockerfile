FROM itsafeaturemythic/mythic_python_base:latest

WORKDIR /Mythic

RUN python3 -m pip install --no-cache-dir --upgrade pip && \
    python3 -m pip install --no-cache-dir pyinstaller donut-shellcode

COPY . /Mythic

CMD ["python3", "/Mythic/main.py"]

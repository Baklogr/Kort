import json
import base64
import ipaddress

from mythic_container.TranslationBase import *


# ID команд — МАЮТЬ збігатися з #define у config.h та Checkin.c
CMD_CHECKIN     = 0xF1
CMD_GET_TASKING = 0x00
CMD_POST_RESPONSE = 0x01


def getBytesWithSize(data: bytes):
    """Читає 4-байтову довжину (LE), потім дані. Повертає (дані, залишок)."""
    size = int.from_bytes(data[0:4], "little")
    data = data[4:]
    return data[:size], data[size:]


def checkIn(data: bytes) -> dict:
    # 1) UUID (36 байт, без довжини)
    uuid = data[:36].decode("cp850")
    data = data[36:]

    # 2) IP-адреси
    numIPs = int.from_bytes(data[0:4], "little")
    data = data[4:]
    IPs = []
    for _ in range(numIPs):
        ip = data[:4]
        data = data[4:]
        IPs.append(str(ipaddress.ip_address(ip)))

    # 3) OS (String з довжиною)
    targetOS, data = getBytesWithSize(data)

    # 4) Arch (Byte)
    arch = data[0]
    data = data[1:]

    # 5) Hostname (String з довжиною)
    hostname, data = getBytesWithSize(data)

    # 6) Username (String з довжиною)
    username, data = getBytesWithSize(data)

    # 7) Domain (WString з довжиною, UTF-16-LE)
    domain_bytes, data = getBytesWithSize(data)
    domain = domain_bytes.decode("utf-16-le")

    # 8) PID (Int32)
    pid = int.from_bytes(data[0:4], "little")
    data = data[4:]

    # 9) Process Name (String з довжиною)
    procname, data = getBytesWithSize(data)

    # 10) External IP (String з довжиною)
    external_ip, data = getBytesWithSize(data)

    # Архітектура: Mythic очікує "x64"/"x86"
    arch_str = "x64" if arch == 9 else "x86"

    return {
        "action": "checkin",
        "uuid": uuid,
        "ips": IPs,
        "os": targetOS.decode("cp850"),
        "architecture": arch_str,
        "hostname": hostname.decode("cp850"),
        "username": username.decode("cp850"),
        "domain": domain,
        "pid": pid,
        "process_name": procname.decode("cp850"),
        "external_ip": external_ip.decode("cp850"),
    }


class KortTranslation(TranslationContainer):
    name = "KortTranslation"
    description = "Translator for the Kort C2 agent (binary Package/Parser protocol)"
    author = "Backlogr"

    async def generate_keys(self, inputMsg: TrGenerateEncryptionKeysMessage) -> TrGenerateEncryptionKeysMessageResponse:
        response = TrGenerateEncryptionKeysMessageResponse(Success=True)
        response.DecryptionKey = b""
        response.EncryptionKey = b""
        return response

    async def translate_to_c2_format(
        self,
        inputMsg: TrMythicC2ToCustomMessageFormatMessage,
    ) -> TrMythicC2ToCustomMessageFormatMessageResponse:
        # Поки заглушка. Тут буде пакування завдань від Mythic
        # у бінарний формат, який очікує агент (get_tasking response).
        response = TrMythicC2ToCustomMessageFormatMessageResponse(Success=True)
        response.Message = b""
        return response

    async def translate_from_c2_format(
        self,
        inputMsg: TrCustomMessageToMythicC2FormatMessage,
    ) -> TrCustomMessageToMythicC2FormatMessageResponse:
        response = TrCustomMessageToMythicC2FormatMessageResponse(Success=True)
        data = inputMsg.Message

        if not data or len(data) == 0:
            response.Success = False
            response.Error = "Empty message"
            return response

        cmd = data[0]

        if cmd == CMD_CHECKIN:
            response.Message = checkIn(data[1:])

        elif cmd == CMD_GET_TASKING:
            # TODO: реалізуємо після tasking loop на стороні агента
            response.Message = {"action": "get_tasking", "tasks": []}

        elif cmd == CMD_POST_RESPONSE:
            # TODO: реалізуємо після post_response
            response.Message = {"action": "post_response"}

        else:
            response.Success = False
            response.Error = f"Unknown command byte: 0x{cmd:02x}"

        return response
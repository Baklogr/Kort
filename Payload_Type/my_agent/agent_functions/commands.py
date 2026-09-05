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


class PingCommand(CommandBase):
    cmd = "ping"
    needs_admin = False
    help_cmd = "ping [host]"
    description = "Send a single ICMP/echo-style ping request to a host."
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
            DisplayParams=taskData.Task.Params,
        )
        return response

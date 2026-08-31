import mythic_container
from Payload_Type.my_agent.agent_functions.builder import MyAgent


# This service is loaded by the Mythic container runtime.
# The `start_and_run_forever` call keeps the service alive and
# handles registration with Mythic.
mythic_container.mythic_service.start_and_run_forever()

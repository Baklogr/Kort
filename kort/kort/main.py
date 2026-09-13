import mythic_container
import asyncio

# Імпортуємо агента (builder.py з agent_functions/)
from agent_functions.builder import *

# Імпортуємо транслятор (translator.py з translator/)
from translator.translator import *

# Запускаємо сервіс Mythic — він зареєструє всі PayloadType і TranslationContainer,
# які були імпортовані вище
mythic_container.mythic_service.start_and_run_forever()
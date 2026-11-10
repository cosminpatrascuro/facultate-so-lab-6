#include <windows.h>
#include <iostream>
#include <string>
#include <cstdlib>

const int LIMITA = 1000;
const char* NUME_MEMORIE = "so_lab6_memorie";
const char* NUME_SEMAFOR = "so_lab6_semafor";

int numara(int id) {
    HANDLE mapare = OpenFileMappingA(FILE_MAP_ALL_ACCESS, FALSE, NUME_MEMORIE);
    HANDLE semafor = OpenSemaphoreA(SEMAPHORE_ALL_ACCESS, FALSE, NUME_SEMAFOR);
    if (mapare == NULL || semafor == NULL) {
        std::cerr << "Deschidere memorie/semafor a esuat: " << GetLastError() << "\n";
        return 1;
    }
    int* memorie = (int*)MapViewOfFile(mapare, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int));

    srand(GetTickCount() ^ GetCurrentProcessId());
    int scrise = 0;

    while (true) {
        WaitForSingleObject(semafor, INFINITE);

        int numar = *memorie;
        if (numar >= LIMITA) {
            std::cout << "Procesul " << id << " a scris " << scrise << " numere\n" << std::flush;
            ReleaseSemaphore(semafor, 1, NULL);
            break;
        }

        while (numar < LIMITA && rand() % 2 == 1) {
            numar++;
            *memorie = numar;
            scrise++;
            std::cout << "Procesul " << id << " (PID " << GetCurrentProcessId() << "): "
                      << numar << "\n" << std::flush;
        }

        ReleaseSemaphore(semafor, 1, NULL);
    }

    UnmapViewOfFile(memorie);
    CloseHandle(mapare);
    CloseHandle(semafor);
    return 0;
}

int main(int argc, char* argv[]) {
    if (argc == 2 && std::string(argv[1]) == "copil")
        return numara(2);

    HANDLE mapare = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE, 0, sizeof(int), NUME_MEMORIE);
    if (mapare == NULL) {
        std::cerr << "CreateFileMapping a esuat: " << GetLastError() << "\n";
        return 1;
    }
    int* memorie = (int*)MapViewOfFile(mapare, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int));
    *memorie = 0;

    HANDLE semafor = CreateSemaphoreA(NULL, 1, 1, NUME_SEMAFOR);
    if (semafor == NULL) {
        std::cerr << "CreateSemaphore a esuat: " << GetLastError() << "\n";
        return 1;
    }

    std::cout << "Parinte (PID " << GetCurrentProcessId() << "): pornesc 2 procese care numara pana la "
              << LIMITA << "\n" << std::flush;

    char exe[MAX_PATH];
    GetModuleFileNameA(NULL, exe, MAX_PATH);
    std::string cmd = std::string("\"") + exe + "\" copil";

    STARTUPINFOA si = {};
    si.cb = sizeof(si);
    PROCESS_INFORMATION pi = {};

    if (!CreateProcessA(NULL, &cmd[0], NULL, NULL, FALSE, 0, NULL, NULL, &si, &pi)) {
        std::cerr << "CreateProcess a esuat: " << GetLastError() << "\n";
        return 1;
    }

    numara(1);
    WaitForSingleObject(pi.hProcess, INFINITE);

    std::cout << "Valoarea finala din memorie: " << *memorie << "\n";

    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);
    UnmapViewOfFile(memorie);
    CloseHandle(mapare);
    CloseHandle(semafor);
    return 0;
}

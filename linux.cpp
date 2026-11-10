#include <iostream>
#include <cstdlib>
#include <ctime>
#include <fcntl.h>
#include <semaphore.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

const int LIMITA = 1000;
const char* NUME_MEMORIE = "/so_lab6_memorie";
const char* NUME_SEMAFOR = "/so_lab6_semafor";

void numara(int id) {
    int fd = shm_open(NUME_MEMORIE, O_RDWR, 0);
    sem_t* semafor = sem_open(NUME_SEMAFOR, 0);
    if (fd < 0 || semafor == SEM_FAILED) {
        perror("deschidere memorie/semafor");
        _exit(1);
    }
    int* memorie = (int*)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);

    srand(time(NULL) ^ getpid());
    int scrise = 0;

    while (true) {
        sem_wait(semafor);

        int numar = *memorie;
        if (numar >= LIMITA) {
            std::cout << "Procesul " << id << " a scris " << scrise << " numere\n" << std::flush;
            sem_post(semafor);
            break;
        }

        while (numar < LIMITA && rand() % 2 == 1) {
            numar++;
            *memorie = numar;
            scrise++;
            std::cout << "Procesul " << id << " (PID " << getpid() << "): " << numar << "\n" << std::flush;
        }

        sem_post(semafor);
    }

    munmap(memorie, sizeof(int));
    sem_close(semafor);
}

int main() {
    shm_unlink(NUME_MEMORIE);
    sem_unlink(NUME_SEMAFOR);

    int fd = shm_open(NUME_MEMORIE, O_CREAT | O_RDWR, 0600);
    if (fd < 0 || ftruncate(fd, sizeof(int)) < 0) {
        perror("shm_open");
        return 1;
    }
    int* memorie = (int*)mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    close(fd);
    *memorie = 0;

    sem_t* semafor = sem_open(NUME_SEMAFOR, O_CREAT, 0600, 1);
    if (semafor == SEM_FAILED) {
        perror("sem_open");
        return 1;
    }
    sem_close(semafor);

    std::cout << "Parinte (PID " << getpid() << "): pornesc 2 procese care numara pana la "
              << LIMITA << "\n" << std::flush;

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        return 1;
    }
    if (pid == 0) {
        numara(2);
        _exit(0);
    }

    numara(1);
    waitpid(pid, nullptr, 0);

    std::cout << "Valoarea finala din memorie: " << *memorie << "\n";

    munmap(memorie, sizeof(int));
    shm_unlink(NUME_MEMORIE);
    sem_unlink(NUME_SEMAFOR);
    return 0;
}

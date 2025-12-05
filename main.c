#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <sys/wait.h>
#include <fcntl.h>

#define SHARED_MEMORY_LOCAL "/shm_local_main"
#define SEM_START_MAIN      "/sem_start_main"
#define SEM_MUTEX_MAIN      "/sem_mutex_main"
#define MSG_KEY_FILE        "/tmp"
#define MSG_KEY_PROJID      255
#define N_CHILDS            4

// Data structure for the shared memory
typedef struct {
    int counter;
} shared_data_t;

// Data structure for the message queue
typedef struct {
    long mtype;
    int pid;
    int value;
} message_t;

// Child process routine
void child_work(int id, sem_t *sem_start, sem_t *sem_mutex, shared_data_t *shm_ptr, int msqid) {
    // Wait for the start signal from parent
    sem_wait(sem_start);

    // Critical Section: Increment shared counter
    sem_wait(sem_mutex);
    shm_ptr->counter++;
    int current_val = shm_ptr->counter;
    printf("Child [%d] (PID: %d) incremented counter to %d\n", id, getpid(), current_val);
    sem_post(sem_mutex);

    // Send message to parent
    message_t msg;
    msg.mtype = 1;
    msg.pid = getpid();
    msg.value = current_val;

    if (msgsnd(msqid, &msg, sizeof(msg) - sizeof(long), 0) == -1) {
        perror("msgsnd child");
        exit(EXIT_FAILURE);
    }

    exit(EXIT_SUCCESS);
}

int main() {
    // 1. Setup Shared Memory
    int shm_fd = shm_open(SHARED_MEMORY_LOCAL, O_CREAT | O_RDWR, 0666);
    if (shm_fd == -1) { perror("shm_open"); exit(EXIT_FAILURE); }
    ftruncate(shm_fd, sizeof(shared_data_t));
    
    shared_data_t *shared_memory = mmap(NULL, sizeof(shared_data_t), PROT_READ | PROT_WRITE, MAP_SHARED, shm_fd, 0);
    if (shared_memory == MAP_FAILED) { perror("mmap"); exit(EXIT_FAILURE); }
    
    // Initialize shared data
    shared_memory->counter = 0;

    // 2. Setup Semaphores
    // sem_start: initialized to 0 (children wait for parent)
    sem_t *sem_start_main = sem_open(SEM_START_MAIN, O_CREAT, 0644, 0);
    // sem_mutex: initialized to 1 (binary mutex for shared memory access)
    sem_t *sem_mutex_main = sem_open(SEM_MUTEX_MAIN, O_CREAT, 0644, 1);

    if (sem_start_main == SEM_FAILED || sem_mutex_main == SEM_FAILED) {
        perror("sem_open");
        exit(EXIT_FAILURE);
    }

    // 3. Setup Message Queue
    key_t key = ftok(MSG_KEY_FILE, MSG_KEY_PROJID);
    if (key == -1) { perror("ftok"); exit(EXIT_FAILURE); }
    
    int msqid = msgget(key, IPC_CREAT | 0666);
    if (msqid == -1) { perror("msgget"); exit(EXIT_FAILURE); }

    // 4. Fork Children
    pid_t pid;
    for (int i = 0; i < N_CHILDS; i++) {
        pid = fork();
        if (pid < 0) { perror("fork"); exit(EXIT_FAILURE); }
        if (pid == 0) {
            child_work(i, sem_start_main, sem_mutex_main, shared_memory, msqid);
        }
    }

    // 5. Parent Logic
    printf("Parent process initialized. Starting synchronization...\n");
    sleep(2); // Simulate preparation time

    // Signal children to start
    printf("Parent broadcasting start signal.\n");
    for (int i = 0; i < N_CHILDS; i++) {
        sem_post(sem_start_main);
    }

    // Wait for messages from children
    for (int i = 0; i < N_CHILDS; i++) {
        message_t msg;
        if (msgrcv(msqid, &msg, sizeof(msg) - sizeof(long), 0, 0) == -1) {
            perror("msgrcv");
        } else {
            printf("Parent received completion from Child PID %d (Counter: %d)\n", msg.pid, msg.value);
        }
    }

    // Wait for all children to exit
    for (int i = 0; i < N_CHILDS; i++) {
        wait(NULL);
    }

    printf("Final Counter Value in Shared Memory: %d\n", shared_memory->counter);

    // 6. Cleanup
    munmap(shared_memory, sizeof(shared_data_t));
    shm_unlink(SHARED_MEMORY_LOCAL);
    sem_close(sem_start_main);
    sem_close(sem_mutex_main);
    sem_unlink(SEM_START_MAIN);
    sem_unlink(SEM_MUTEX_MAIN);
    msgctl(msqid, IPC_RMID, NULL);

    return 0;
}

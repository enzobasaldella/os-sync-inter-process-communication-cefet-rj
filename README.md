## Process Synchronization & IPC

Academic project developed for the Operating Systems course.

This project demonstrates Inter-Process Communication (IPC) and synchronization mechanisms in a Linux environment using C. It orchestrates a parent process and multiple child processes interacting through Shared Memory, Semaphores, and Message Queues.

# Features

- **POSIX Shared Memory:** Shared state management between processes.

- **POSIX Semaphores:**
  1. Signaling: Parent process controls the start execution of children.
  2. Mutex: Binary semaphore ensures atomic access to shared memory.

- **System V Message Queues:** Communication channel for children to report status back to the parent.

- **Process Management:** Usage of fork() and wait() for lifecycle control.

# How to Compile and Run

Compile the source code using Make:

`make`

Run the executable:

`./ipc_sync`

# Execution Flow

1 - Parent initializes Shared Memory, Semaphores, and Message Queue.

2 - Parent spawns 4 child processes.

3 - Children wait on a semaphore (sem_wait) for a broadcast signal.

4 - Parent signals children to start (sem_post).

5 - Children increment a shared counter protected by a mutex.

6 - Children send a completion message via Message Queue.

7 - Parent receives messages, waits for termination, and performs cleanup.

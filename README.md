# Process Synchronization and Inter-Process Communication

A C program demonstrating process synchronization and inter-process communication (IPC) on Linux. Developed for an Operating Systems course, it coordinates a parent process and four child processes using POSIX shared memory, named semaphores, and a System V message queue.

> [!NOTE]
> Each child increments the same shared counter exactly once. Semaphore protection ensures that the final value is `4`, although the order in which children execute and report completion is not deterministic.

## Features

- Four child processes created with `fork()`
- POSIX shared memory for a counter accessible to all processes
- A start semaphore that keeps children waiting until the parent releases them
- A binary semaphore for mutual exclusion during counter updates
- System V messages carrying each child's PID and observed counter value
- Process termination handling with `wait()` and removal of IPC resources on normal completion

## IPC mechanisms

| Mechanism | Main functions | Purpose |
| --- | --- | --- |
| Shared memory | `shm_open()`, `ftruncate()`, `mmap()` | Store the counter across processes |
| Named semaphores | `sem_open()`, `sem_wait()`, `sem_post()` | Coordinate startup and protect the critical section |
| Message queue | `ftok()`, `msgget()`, `msgsnd()`, `msgrcv()` | Deliver completion messages to the parent |
| Process management | `fork()`, `wait()` | Create child processes and wait for their termination |

## Execution flow

1. The parent creates and maps shared memory, initializes the counter to zero, and opens two named semaphores.
2. It creates a System V message queue and forks four children.
3. Each child blocks on the start semaphore.
4. After a short delay, the parent calls `sem_post()` four times, releasing the children.
5. Each child locks the mutex semaphore, increments the shared counter, records its value, and unlocks the semaphore.
6. Each child sends a completion message containing its PID and counter value.
7. The parent receives four messages, waits for the children to exit, prints the final counter, and removes the IPC resources.

## Requirements

- Linux with POSIX shared memory, POSIX semaphores, and System V IPC support
- GCC
- GNU Make

## Build and run

From the repository root:

```bash
make
./ipc_sync
```

Alternatively, compile and execute in one step:

```bash
make run
```

A successful run prints one increment message per child, four completion messages received by the parent, and:

```text
Final Counter Value in Shared Memory: 4
```

The order of the child messages can change between executions.

Remove the generated executable and object file with:

```bash
make clean
```

> [!IMPORTANT]
> The program removes its named shared memory, named semaphores, and System V message queue during normal shutdown. If it is interrupted before cleanup, IPC objects may remain on the system and require manual removal.

## Project structure

```text
.
├── main.c       # Process creation, synchronization, messaging, and cleanup
├── Makefile     # Build, run, and clean targets
└── README.md    # Project documentation
```

## Scope and limitations

- All processes run on one Linux machine; the project demonstrates local IPC rather than communication over a network.
- The number of children is fixed at four in `N_CHILDS`.
- The project is an educational example, not a fault-tolerant IPC service. Some system-call and semaphore-operation failures are not fully recovered.

# Readers–Writers Toolkit

A terminal-based Linux simulator written in C++17 that demonstrates concurrent
synchronization algorithms using POSIX threads and semaphores — with a live,
colour-coded dashboard showing every thread's state in real time.

---

## Quick Start

```bash
# 1. Build
make

# 2. Run with defaults (3 readers, 2 writers, reader-priority mode)
make run

# 3. Run in writer-priority mode with a trace log
./rw_simulator -r 4 -w 3 -m writer -l run.log

# 4. Custom thread counts
./rw_simulator -r 5 -w 2 -i 6 -m reader
```

---

## CLI Flags

| Flag | Default | Description |
|------|---------|-------------|
| `-r N` | 3 | Number of reader threads |
| `-w N` | 2 | Number of writer threads |
| `-i N` | 4 | Iterations (read/write cycles) per thread |
| `-m reader\|writer` | `reader` | Synchronization mode |
| `-l FILE` | *(none)* | Write timestamped trace log to `FILE` |
| `-h` | — | Print help and exit |

---

## Synchronization Algorithms

### Reader-Priority  (`-m reader`)

Based on the classic Courtois et al. (1971) solution.

```
readers_count  — tracks concurrent readers inside
rc_mutex       — protects readers_count
resource       — binary semaphore; first reader grabs it (locking out
                 writers); last reader releases it
```

*Effect:* As long as readers keep arriving, writers starve.
Good for read-heavy workloads with infrequent writes.

### Writer-Priority  (`-m writer`)

Extends the above with two extra primitives:

```
writers_count    — tracks pending/active writers
wc_mutex         — protects writers_count
read_try_mutex   — held by the writer group whenever writers_count > 0;
                   new readers must lock this first, so they block until
                   no writers are waiting
```

*Effect:* Once a writer arrives, no new reader can enter the critical
section.  Writers get access as soon as current readers finish.
Prevents writer starvation.

---

## Project Structure

```
rw_toolkit/
├── main.cpp       Full simulator source
├── Makefile       Build rules
└── README.md      This file
```

---

## Dependencies

| Library | Header | Purpose |
|---------|--------|---------|
| POSIX Threads | `<pthread.h>` | Thread creation, mutexes |
| POSIX Semaphores | `<semaphore.h>` | Binary semaphore for resource |
| POSIX unistd | `<unistd.h>` | `usleep()` for artificial delays |
| C++17 std | `<chrono>` etc. | Timing, containers |

Everything ships with standard Ubuntu/Debian via `build-essential`.

---

## Live Dashboard — Legend

```
░░░ WAITING   — thread requested lock, blocked
▓▓▓ READING   — thread is inside the read critical section
███ WRITING   — thread holds exclusive write lock
─── IDLE/DONE — thread is resting between cycles or finished
```

Readers appear in **cyan**, writers in **red**.

---

## Safety Guarantees

| Property | How it's achieved |
|----------|-------------------|
| No race condition | All accesses to `value` happen inside a lock |
| No deadlock | Lock ordering is fixed: `wc_mutex → read_try_mutex → rc_mutex → resource`; no circular wait |
| Mutual exclusion for writers | `resource` semaphore is always at most 1 |
| Reader concurrency | Multiple threads may hold the read lock simultaneously |

---

## Log File Format

```
[    12.34 ms] Reader 1   acquired read lock  [shared value = 0]
[    15.01 ms] Writer 4   acquired write lock  [shared value → 1]
[   318.45 ms] Writer 4   released write lock
```

Timestamps are milliseconds since simulation start.

---

## Correctness Check

When the simulation ends the program prints:

```
Final shared value : 8
Expected writes    : 8   ✓ CORRECT
```

Since each writer increments `value` exactly once per iteration, the
final value must equal `n_writers × iterations`.  A mismatch means
there is a race condition in the lock logic.

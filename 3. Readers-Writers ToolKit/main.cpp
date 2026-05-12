/*
 * ╔══════════════════════════════════════════════════════════════════╗
 * ║          Readers–Writers Toolkit  ─  rw_simulator               ║
 * ║  Demonstrates reader-priority and writer-priority algorithms     ║
 * ║  with live terminal visualization and file logging.              ║
 * ╚══════════════════════════════════════════════════════════════════╝
 *
 * Build:  make
 * Usage:  ./rw_simulator -r 5 -w 2 -m writer -l run.log
 *
 * Options:
 *   -r N     Number of reader threads  (default: 3)
 *   -w N     Number of writer threads  (default: 2)
 *   -i N     Iterations per thread     (default: 4)
 *   -m MODE  Sync mode: reader | writer (default: reader)
 *   -l FILE  Write events to log file  (optional)
 *   -h       Show this help
 */

#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <chrono>
#include <string>
#include <vector>
#include <atomic>

// ════════════════════════════════════════════════════════════════════
//  ANSI colour helpers
// ════════════════════════════════════════════════════════════════════
#define CLR_RESET   "\033[0m"
#define CLR_BOLD    "\033[1m"
#define CLR_GREY    "\033[90m"
#define CLR_YELLOW  "\033[33m"
#define CLR_GREEN   "\033[32m"
#define CLR_RED     "\033[31m"
#define CLR_CYAN    "\033[36m"
#define CLR_MAGENTA "\033[35m"
#define CLR_WHITE   "\033[97m"

// ════════════════════════════════════════════════════════════════════
//  Enums
// ════════════════════════════════════════════════════════════════════
enum ThreadState { IDLE, WAITING, READING, WRITING, DONE };
enum Mode        { READER_PRIORITY, WRITER_PRIORITY };

static const char* STATE_LABEL[] = { "IDLE   ", "WAITING", "READING", "WRITING", "DONE   " };
static const char* STATE_COLOR[] = { CLR_GREY, CLR_YELLOW, CLR_GREEN, CLR_RED, CLR_GREY };

// ════════════════════════════════════════════════════════════════════
//  Shared resource + ALL synchronization primitives
//
//  Reader-priority (classic Courtois et al.):
//    readers_count  ─ how many readers are currently inside
//    rc_mutex       ─ guards readers_count
//    resource       ─ semaphore that serialises writer access;
//                     first reader locks it, last reader unlocks it
//
//  Writer-priority extras:
//    writers_count  ─ how many writers are currently waiting/writing
//    wc_mutex       ─ guards writers_count
//    read_try_mutex ─ held by writers while writers_count > 0;
//                     new readers must grab it first, so they block
//                     until no writers are pending
// ════════════════════════════════════════════════════════════════════
struct SharedData {
    int  value;            // The shared resource (a counter)

    // Reader-priority primitives
    sem_t           resource;
    pthread_mutex_t rc_mutex;
    int             readers_count;

    // Writer-priority extras
    pthread_mutex_t wc_mutex;
    pthread_mutex_t read_try_mutex;
    int             writers_count;

    Mode mode;
};

// ════════════════════════════════════════════════════════════════════
//  Per-thread bookkeeping visible to the visualizer
// ════════════════════════════════════════════════════════════════════
struct ThreadInfo {
    int             id;
    bool            is_writer;
    ThreadState     state;
    pthread_mutex_t state_lock;   // serialises state read/write
    SharedData*     sd;
    int             iterations;
    FILE*           log_fp;
    pthread_mutex_t* log_lock;
};

// ════════════════════════════════════════════════════════════════════
//  Globals for the live dashboard
// ════════════════════════════════════════════════════════════════════
static std::vector<ThreadInfo*> g_threads;
static volatile bool            g_running = true;
static std::chrono::steady_clock::time_point g_start;

static double elapsed_ms() {
    auto now = std::chrono::steady_clock::now();
    return std::chrono::duration<double, std::milli>(now - g_start).count();
}

// ════════════════════════════════════════════════════════════════════
//  State helpers (thread-safe)
// ════════════════════════════════════════════════════════════════════
static void set_state(ThreadInfo* ti, ThreadState s) {
    pthread_mutex_lock(&ti->state_lock);
    ti->state = s;
    pthread_mutex_unlock(&ti->state_lock);
}

static ThreadState get_state(ThreadInfo* ti) {
    pthread_mutex_lock(&ti->state_lock);
    ThreadState s = ti->state;
    pthread_mutex_unlock(&ti->state_lock);
    return s;
}

// ════════════════════════════════════════════════════════════════════
//  Logging helper
// ════════════════════════════════════════════════════════════════════
static void log_event(ThreadInfo* ti, const char* fmt, ...) {
    if (!ti->log_fp) return;

    char msg[512];
    va_list ap;
    va_start(ap, fmt);
    vsnprintf(msg, sizeof(msg), fmt, ap);
    va_end(ap);

    pthread_mutex_lock(ti->log_lock);
    fprintf(ti->log_fp, "[%9.2f ms] %s %-2d  %s\n",
            elapsed_ms(),
            ti->is_writer ? "Writer" : "Reader",
            ti->id,
            msg);
    fflush(ti->log_fp);
    pthread_mutex_unlock(ti->log_lock);
}

// ════════════════════════════════════════════════════════════════════
//  ── READER-PRIORITY lock / unlock ────────────────────────────────
//
//  Multiple readers enter simultaneously.
//  The first arriving reader locks the resource semaphore (blocking
//  any writer).  Each subsequent reader just increments readers_count.
//  The last reader to leave posts the semaphore, allowing a waiting
//  writer to proceed.
// ════════════════════════════════════════════════════════════════════
static void rp_read_lock(SharedData* sd) {
    pthread_mutex_lock(&sd->rc_mutex);
    sd->readers_count++;
    if (sd->readers_count == 1)   // first reader
        sem_wait(&sd->resource);  // lock out writers
    pthread_mutex_unlock(&sd->rc_mutex);
}

static void rp_read_unlock(SharedData* sd) {
    pthread_mutex_lock(&sd->rc_mutex);
    sd->readers_count--;
    if (sd->readers_count == 0)   // last reader
        sem_post(&sd->resource);  // release writers
    pthread_mutex_unlock(&sd->rc_mutex);
}

static void rp_write_lock(SharedData* sd)   { sem_wait(&sd->resource); }
static void rp_write_unlock(SharedData* sd) { sem_post(&sd->resource); }

// ════════════════════════════════════════════════════════════════════
//  ── WRITER-PRIORITY lock / unlock ────────────────────────────────
//
//  When the first writer arrives it grabs read_try_mutex, preventing
//  any new readers from entering the critical section while writers
//  are queued.  Readers currently inside finish, but newcomers block.
//  The last writer releases read_try_mutex, unblocking queued readers.
// ════════════════════════════════════════════════════════════════════
static void wp_read_lock(SharedData* sd) {
    // Block here if any writer is pending (read_try_mutex held by writers)
    pthread_mutex_lock(&sd->read_try_mutex);

    pthread_mutex_lock(&sd->rc_mutex);
    sd->readers_count++;
    if (sd->readers_count == 1)
        sem_wait(&sd->resource);  // first reader locks resource
    pthread_mutex_unlock(&sd->rc_mutex);

    pthread_mutex_unlock(&sd->read_try_mutex);
}

static void wp_read_unlock(SharedData* sd) {
    pthread_mutex_lock(&sd->rc_mutex);
    sd->readers_count--;
    if (sd->readers_count == 0)
        sem_post(&sd->resource);  // last reader releases resource
    pthread_mutex_unlock(&sd->rc_mutex);
}

static void wp_write_lock(SharedData* sd) {
    pthread_mutex_lock(&sd->wc_mutex);
    sd->writers_count++;
    if (sd->writers_count == 1)
        pthread_mutex_lock(&sd->read_try_mutex);  // block new readers
    pthread_mutex_unlock(&sd->wc_mutex);

    sem_wait(&sd->resource);  // wait for exclusive access
}

static void wp_write_unlock(SharedData* sd) {
    sem_post(&sd->resource);  // release exclusive access

    pthread_mutex_lock(&sd->wc_mutex);
    sd->writers_count--;
    if (sd->writers_count == 0)
        pthread_mutex_unlock(&sd->read_try_mutex);  // allow readers again
    pthread_mutex_unlock(&sd->wc_mutex);
}

// ════════════════════════════════════════════════════════════════════
//  Reader thread routine
// ════════════════════════════════════════════════════════════════════
static void* reader_routine(void* arg) {
    ThreadInfo* ti = (ThreadInfo*)arg;
    SharedData* sd = ti->sd;

    for (int i = 0; i < ti->iterations; i++) {
        // ── Request access ──────────────────────────────────────────
        set_state(ti, WAITING);
        log_event(ti, "requesting read lock");

        if (sd->mode == READER_PRIORITY) rp_read_lock(sd);
        else                             wp_read_lock(sd);

        // ── Inside critical section (reading) ───────────────────────
        set_state(ti, READING);
        log_event(ti, "acquired read lock  [shared value = %d]", sd->value);

        usleep((100 + rand() % 200) * 1000);   // simulate 100–300 ms of reading

        if (sd->mode == READER_PRIORITY) rp_read_unlock(sd);
        else                             wp_read_unlock(sd);

        // ── Released ────────────────────────────────────────────────
        log_event(ti, "released read lock");
        set_state(ti, IDLE);

        usleep((50 + rand() % 150) * 1000);    // rest between cycles
    }

    set_state(ti, DONE);
    log_event(ti, "all iterations complete");
    return nullptr;
}

// ════════════════════════════════════════════════════════════════════
//  Writer thread routine
// ════════════════════════════════════════════════════════════════════
static void* writer_routine(void* arg) {
    ThreadInfo* ti = (ThreadInfo*)arg;
    SharedData* sd = ti->sd;

    for (int i = 0; i < ti->iterations; i++) {
        // ── Request exclusive access ────────────────────────────────
        set_state(ti, WAITING);
        log_event(ti, "requesting write lock");

        if (sd->mode == READER_PRIORITY) rp_write_lock(sd);
        else                             wp_write_lock(sd);

        // ── Inside critical section (writing) ───────────────────────
        set_state(ti, WRITING);
        sd->value++;                            // mutate the shared resource
        log_event(ti, "acquired write lock  [shared value → %d]", sd->value);

        usleep((150 + rand() % 200) * 1000);   // simulate 150–350 ms of writing

        if (sd->mode == READER_PRIORITY) rp_write_unlock(sd);
        else                             wp_write_unlock(sd);

        // ── Released ────────────────────────────────────────────────
        log_event(ti, "released write lock");
        set_state(ti, IDLE);

        usleep((80 + rand() % 120) * 1000);
    }

    set_state(ti, DONE);
    log_event(ti, "all iterations complete");
    return nullptr;
}

// ════════════════════════════════════════════════════════════════════
//  Live terminal dashboard (runs on its own thread, refreshes ~10 Hz)
// ════════════════════════════════════════════════════════════════════
static void* viz_routine(void* arg) {
    SharedData* sd = (SharedData*)arg;

    // Count total iterations from the first thread (all share the same value)
    int total_iters = g_threads.empty() ? 1 : g_threads[0]->iterations;

    while (g_running) {
        // Move cursor to top-left (no flicker clear)
        printf("\033[H");

        // ── Header bar ─────────────────────────────────────────────
        printf(CLR_BOLD CLR_CYAN
               "╔══════════════════════════════════════════════════════════╗\n"
               "║       Readers–Writers Toolkit  ─  Live Dashboard         ║\n"
               "╚══════════════════════════════════════════════════════════╝\n"
               CLR_RESET);

        printf(CLR_BOLD "  Mode: " CLR_MAGENTA "%s" CLR_RESET
               CLR_BOLD "   Shared value: " CLR_WHITE "%d" CLR_RESET
               CLR_BOLD "   Elapsed: " CLR_GREY "%.0f ms\n\n" CLR_RESET,
               sd->mode == READER_PRIORITY ? "Reader-Priority" : "Writer-Priority",
               sd->value,
               elapsed_ms());

        // ── Column header ──────────────────────────────────────────
        printf(CLR_BOLD "  %-8s %-6s %-9s  Timeline\n" CLR_RESET,
               "Thread", "Type", "State");
        printf("  %s\n", std::string(58, '-').c_str());

        // ── One row per thread ─────────────────────────────────────
        for (auto* t : g_threads) {
            ThreadState s   = get_state(t);
            const char* col = STATE_COLOR[s];
            const char* lbl = STATE_LABEL[s];
            const char* typ = t->is_writer ? "Writer" : "Reader";

            // Bar: width proportional to "activity level"
            int bar_w = 0;
            const char* bar_ch = "─";
            if      (s == READING) { bar_w = 24; bar_ch = "▓"; }
            else if (s == WRITING) { bar_w = 24; bar_ch = "█"; }
            else if (s == WAITING) { bar_w = 12; bar_ch = "░"; }

            std::string bar;
            for (int b = 0; b < bar_w; b++) bar += bar_ch;

            printf("  %s%-2d%-6s  %-8s %s%-9s%s  %s%s%s\n",
                   t->is_writer ? CLR_RED : CLR_CYAN,
                   t->id, CLR_RESET,
                   typ,
                   col, lbl, CLR_RESET,
                   col, bar.c_str(), CLR_RESET);
        }

        // ── Legend ─────────────────────────────────────────────────
        printf("\n  " CLR_GREY
               "Legend: "
               CLR_YELLOW  "░░░ WAITING  "
               CLR_GREEN   "▓▓▓ READING  "
               CLR_RED     "███ WRITING  "
               CLR_GREY    "─── IDLE/DONE\n"
               CLR_RESET);

        fflush(stdout);
        usleep(100000);   // 100 ms refresh
    }
    (void)total_iters;
    return nullptr;
}

// ════════════════════════════════════════════════════════════════════
//  Usage
// ════════════════════════════════════════════════════════════════════
static void print_usage(const char* prog) {
    fprintf(stderr,
        "\nUsage: %s [options]\n\n"
        "  -r N       Number of reader threads   (default: 3)\n"
        "  -w N       Number of writer threads   (default: 2)\n"
        "  -i N       Iterations per thread      (default: 4)\n"
        "  -m MODE    Sync mode: reader | writer (default: reader)\n"
        "  -l FILE    Write trace log to FILE    (optional)\n"
        "  -h         Show this help and exit\n\n"
        "Examples:\n"
        "  %s -r 5 -w 2 -m writer -l run.log\n"
        "  %s -r 3 -w 3 -m reader\n\n",
        prog, prog, prog);
}

// ════════════════════════════════════════════════════════════════════
//  main
// ════════════════════════════════════════════════════════════════════
int main(int argc, char* argv[]) {

    // ── Defaults ──────────────────────────────────────────────────
    int         n_readers  = 3;
    int         n_writers  = 2;
    int         iterations = 4;
    Mode        mode       = READER_PRIORITY;
    const char* log_path   = nullptr;

    // ── CLI argument parsing ──────────────────────────────────────
    for (int i = 1; i < argc; i++) {
        if      (!strcmp(argv[i], "-r") && i+1 < argc) n_readers  = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-w") && i+1 < argc) n_writers  = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-i") && i+1 < argc) iterations = atoi(argv[++i]);
        else if (!strcmp(argv[i], "-l") && i+1 < argc) log_path   = argv[++i];
        else if (!strcmp(argv[i], "-m") && i+1 < argc) {
            ++i;
            if      (!strcmp(argv[i], "writer")) mode = WRITER_PRIORITY;
            else if (!strcmp(argv[i], "reader")) mode = READER_PRIORITY;
            else {
                fprintf(stderr, "Unknown mode '%s'. Use 'reader' or 'writer'.\n", argv[i]);
                return 1;
            }
        }
        else if (!strcmp(argv[i], "-h")) { print_usage(argv[0]); return 0; }
        else {
            fprintf(stderr, "Unknown flag '%s'. Use -h for help.\n", argv[i]);
            return 1;
        }
    }

    // Sanity checks
    if (n_readers < 1 || n_writers < 1 || iterations < 1) {
        fprintf(stderr, "Counts must be >= 1.\n");
        return 1;
    }

    // ── Open log file ─────────────────────────────────────────────
    FILE*           log_fp   = nullptr;
    pthread_mutex_t log_lock = PTHREAD_MUTEX_INITIALIZER;

    if (log_path) {
        log_fp = fopen(log_path, "w");
        if (!log_fp) { perror("fopen"); return 1; }
        fprintf(log_fp,
                "=== Readers-Writers Toolkit ─ Trace Log ===\n"
                "Mode       : %s\n"
                "Readers    : %d\n"
                "Writers    : %d\n"
                "Iterations : %d\n"
                "==========================================\n\n",
                mode == READER_PRIORITY ? "reader-priority" : "writer-priority",
                n_readers, n_writers, iterations);
    }

    // ── Initialise shared data + synchronization primitives ───────
    SharedData sd;
    memset(&sd, 0, sizeof(sd));
    sd.mode = mode;

    sem_init(&sd.resource,        0, 1);   // binary semaphore
    pthread_mutex_init(&sd.rc_mutex,       nullptr);
    pthread_mutex_init(&sd.wc_mutex,       nullptr);
    pthread_mutex_init(&sd.read_try_mutex, nullptr);

    // ── Build per-thread info structs ─────────────────────────────
    int total = n_readers + n_writers;
    std::vector<ThreadInfo> infos(total);

    int id = 1;
    for (int i = 0; i < n_readers; i++, id++) {
        ThreadInfo& t = infos[i];
        t.id = id;  t.is_writer = false;  t.state = IDLE;
        pthread_mutex_init(&t.state_lock, nullptr);
        t.sd = &sd;  t.iterations = iterations;
        t.log_fp = log_fp;  t.log_lock = &log_lock;
        g_threads.push_back(&t);
    }
    for (int i = 0; i < n_writers; i++, id++) {
        ThreadInfo& t = infos[n_readers + i];
        t.id = id;  t.is_writer = true;  t.state = IDLE;
        pthread_mutex_init(&t.state_lock, nullptr);
        t.sd = &sd;  t.iterations = iterations;
        t.log_fp = log_fp;  t.log_lock = &log_lock;
        g_threads.push_back(&t);
    }

    // ── Clear screen and show startup banner ──────────────────────
    printf("\033[2J\033[H");
    printf(CLR_BOLD CLR_CYAN
           "╔══════════════════════════════════════════════════════════╗\n"
           "║         Readers–Writers Toolkit  ─  Starting Up          ║\n"
           "╚══════════════════════════════════════════════════════════╝\n"
           CLR_RESET
           CLR_BOLD "  Mode       : " CLR_MAGENTA "%s\n" CLR_RESET
           CLR_BOLD "  Readers    : " CLR_CYAN    "%d\n" CLR_RESET
           CLR_BOLD "  Writers    : " CLR_RED     "%d\n" CLR_RESET
           CLR_BOLD "  Iterations : " CLR_WHITE   "%d\n" CLR_RESET
           CLR_BOLD "  Log file   : " CLR_GREY    "%s\n\n" CLR_RESET,
           mode == READER_PRIORITY ? "Reader-Priority" : "Writer-Priority",
           n_readers, n_writers, iterations,
           log_path ? log_path : "(none)");
    fflush(stdout);
    usleep(600000);   // brief pause so banner is readable

    // ── Record start time ─────────────────────────────────────────
    g_start = std::chrono::steady_clock::now();

    // ── Start the visualization thread ────────────────────────────
    pthread_t viz_tid;
    pthread_create(&viz_tid, nullptr, viz_routine, &sd);

    // ── Spawn reader threads ──────────────────────────────────────
    std::vector<pthread_t> tids(total);
    for (int i = 0; i < n_readers; i++)
        pthread_create(&tids[i], nullptr, reader_routine, &infos[i]);

    // ── Spawn writer threads ──────────────────────────────────────
    for (int i = 0; i < n_writers; i++)
        pthread_create(&tids[n_readers + i], nullptr, writer_routine, &infos[n_readers + i]);

    // ── Wait for all worker threads to finish ─────────────────────
    for (int i = 0; i < total; i++)
        pthread_join(tids[i], nullptr);

    // ── Stop the visualizer cleanly ───────────────────────────────
    g_running = false;
    pthread_join(viz_tid, nullptr);

    // ── Final summary ─────────────────────────────────────────────
    int expected_writes = n_writers * iterations;
    printf("\n\n" CLR_BOLD CLR_GREEN
           "  ✓ Simulation complete  (%.0f ms total)\n" CLR_RESET,
           elapsed_ms());
    printf(CLR_BOLD
           "  Final shared value : %d\n"
           "  Expected writes    : %d   %s\n"
           CLR_RESET "\n",
           sd.value,
           expected_writes,
           sd.value == expected_writes
               ? CLR_GREEN "✓ CORRECT" CLR_RESET
               : CLR_RED   "✗ MISMATCH — check your locks!" CLR_RESET);

    // ── Cleanup ───────────────────────────────────────────────────
    sem_destroy(&sd.resource);
    pthread_mutex_destroy(&sd.rc_mutex);
    pthread_mutex_destroy(&sd.wc_mutex);
    pthread_mutex_destroy(&sd.read_try_mutex);
    for (auto& t : infos)
        pthread_mutex_destroy(&t.state_lock);
    pthread_mutex_destroy(&log_lock);

    if (log_fp) {
        fprintf(log_fp,
                "\n=== Simulation complete ===\n"
                "Final shared value : %d\n"
                "Expected writes    : %d\n"
                "Result             : %s\n",
                sd.value, expected_writes,
                sd.value == expected_writes ? "CORRECT" : "MISMATCH");
        fclose(log_fp);
        printf("  Trace log written to: %s\n\n", log_path);
    }

    return 0;
}

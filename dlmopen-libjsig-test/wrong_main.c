#define _GNU_SOURCE

#include "wrong_main.h"
#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

typedef void (*cob_init_func)(int, char **);
typedef int (*cob_tidy_func)(void);
typedef int (*cob_call_with_exception_check_func)(const char *, const int, void **);

#define NUM_COBOL_THREADS 4
static void *libcobs[NUM_COBOL_THREADS];
static pthread_t threads[NUM_COBOL_THREADS];
static char thread_ready[NUM_COBOL_THREADS];
static char thread_allowed_to_exit[NUM_COBOL_THREADS];
static int selected_thread_for_signal = -1;

#define RESOLVE_SYMBOL(symbol, type)                                            \
  type symbol = (type) dlsym(libCobHandle, #symbol);                            \
  if (!symbol) {                                                                \
    fprintf(stderr, "Failed to load %s: %s\n", #symbol, dlerror());             \
    dlclose(libCobHandle);                                                      \
    exit(1);                                                                    \
  };

static void initialize_cobol_runtimes()
{
    for (int i = 0; i < NUM_COBOL_THREADS; i++) {
        void *libCobHandle = dlmopen(LM_ID_NEWLM, "libcob.so", RTLD_NOW);
        if (libCobHandle == NULL) {
            perror("Failed to open libcob with dlmopen!\n");
            exit(20);
        }
        libcobs[i] = libCobHandle;
    }
}

static void cleanup_cobol_runtimes()
{
    for (int i = 0; i < NUM_COBOL_THREADS; i++) {
        thread_allowed_to_exit[i]= 'Y';
        if (i != selected_thread_for_signal && pthread_join(threads[i], NULL) != 0) {
            perror("Cannot gracefully stop thread after sending signal!\n");
#ifndef SHARED_LIB
            exit(1);
#endif
        }
    }
    for (int i = 0; i < NUM_COBOL_THREADS; i++) {
        if (dlclose(libcobs[i]) != 0) {
            perror("Failed to close libcob!\n");
            exit(1);
        }
    }
}

static void *cobol_thread(void *ptr)
{
    intptr_t thread_id = (intptr_t) ptr;
    char thread_id_str[32];
    sprintf(thread_id_str, "%ld", thread_id);
    printf("Hello from thread %ld!\n", thread_id);
    void *libCobHandle = libcobs[thread_id];
    RESOLVE_SYMBOL(cob_init, cob_init_func);
    RESOLVE_SYMBOL(cob_call_with_exception_check, cob_call_with_exception_check_func);
    RESOLVE_SYMBOL(cob_tidy, cob_tidy_func);
    cob_init(0, NULL);
    void *args[3];
    args[0] = (void *) thread_id_str;
    args[1] = (void *) &thread_ready[thread_id];
    args[2] = (void *) &thread_allowed_to_exit[thread_id];
    int result = cob_call_with_exception_check("COBTEST", 3, args);
    if (selected_thread_for_signal == thread_id && result != -3) {
        printf("COBOL thread %ld did not exit from the signal handler!\n", thread_id);
#ifndef SHARED_LIB
        exit(1);
#endif
    }
    if (selected_thread_for_signal != thread_id && result != 1) {
        printf("COBOL thread %ld did not exit gracefully!\n", thread_id);
#ifndef SHARED_LIB
        exit(1);
#endif
    }
    cob_tidy();
    printf("Thread %ld received result: %d\n", thread_id, result);
    return NULL;
}

static void initialize_cobol_threads() {
    for (intptr_t i = 0; i < NUM_COBOL_THREADS; i++) {
        if (pthread_create(&threads[i], NULL, cobol_thread, (void *) i) != 0) {
            perror("Cannot create COBOL thread!\n");
        }
    }
}

static int all_cobol_threads_ready()
{
    for (int i = 0; i < NUM_COBOL_THREADS; i++) {
        if (thread_ready[i] != 'Y') {
            return 0;
        }
    }
    return 1;
}

int run_cobol_threads_default()
{
    memset(&thread_ready, 0, sizeof(thread_ready));
    memset(&thread_allowed_to_exit, 0, sizeof(thread_allowed_to_exit));
    initialize_cobol_runtimes();
    initialize_cobol_threads();
    while (all_cobol_threads_ready() == 0) {
        sleep(1);
    }
#ifndef SHARED_LIB
    srand(time(NULL));
    selected_thread_for_signal = rand() % NUM_COBOL_THREADS;
    printf("All cobol threads are ready! Sending segfault to thread %d\n", selected_thread_for_signal);
    if (pthread_kill(threads[selected_thread_for_signal], SIGSEGV) != 0) {
        perror("Cannot send signal to cobol thread!\n");
        exit(1);
    }
    if (pthread_join(threads[selected_thread_for_signal], NULL) != 0) {
        perror("Cannot join the thread!\n");
        exit(1);
    }
    cleanup_cobol_runtimes(selected_thread_for_signal);
#else
    sleep(1);
#endif
    return 0;
}

#ifndef SHARED_LIB
int main()
{
    return run_cobol_threads_default();
}
#endif
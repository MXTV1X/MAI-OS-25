#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <time.h>
#include <unistd.h>
#include <getopt.h>

#define THREAD_STACK_SIZE (64 * 1024)

long global_match_count = 0; 
pthread_mutex_t count_mutex;

typedef struct {
    const char* text;
    const char* pattern;
    int start_index;
    int end_index;
    int pattern_len;
} ThreadData;

double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

void* naive_search_thread(void* arg) {
    ThreadData* data = (ThreadData*)arg;
    long local_count = 0;

    for (int i = data->start_index; i <= data->end_index; i++) {
        int j;
        for (j = 0; j < data->pattern_len; j++) {
            if (data->text[i + j] != data->pattern[j]) break;
        }
        if (j == data->pattern_len) {
            local_count++;
        }
    }

    pthread_mutex_lock(&count_mutex);
    global_match_count += local_count;
    pthread_mutex_unlock(&count_mutex);

    return NULL;
}

int main(int argc, char* argv[]) {
    int num_threads = 1;
    int opt;

    while ((opt = getopt(argc, argv, "t:")) != -1) {
        if (opt == 't') num_threads = atoi(optarg);
    }
    if (num_threads < 1) num_threads = 1;

    pthread_mutex_init(&count_mutex, NULL);

    size_t text_len = 50 * 1024 * 1024;
    char* text = malloc(text_len + 1);
    memset(text, 'A', text_len);
    const char* pattern = "ABACABA";
    int pat_len = strlen(pattern);
    for (int i = 0; i < 1000; i++) {
        memcpy(text + (rand() % (text_len - pat_len)), pattern, pat_len);
    }
    text[text_len] = '\0';

    global_match_count = 0;
    ThreadData seq_data = {text, pattern, 0, (int)(text_len - pat_len), pat_len};
    double start_t1 = get_time();
    naive_search_thread(&seq_data);
    double end_t1 = get_time();
    double t1 = end_t1 - start_t1;
    long seq_matches = global_match_count;

    global_match_count = 0;
    pthread_t* threads = malloc(num_threads * sizeof(pthread_t));
    ThreadData* t_args = malloc(num_threads * sizeof(ThreadData));
    
    pthread_attr_t attr;
    pthread_attr_init(&attr);
    pthread_attr_setstacksize(&attr, THREAD_STACK_SIZE);

    int positions = text_len - pat_len + 1;
    int chunk = positions / num_threads;

    double start_tn = get_time();
    for (int i = 0; i < num_threads; i++) {
        t_args[i].text = text;
        t_args[i].pattern = pattern;
        t_args[i].pattern_len = pat_len;
        t_args[i].start_index = i * chunk;
        t_args[i].end_index = (i == num_threads - 1) ? (positions - 1) : ((i + 1) * chunk - 1);
        pthread_create(&threads[i], &attr, naive_search_thread, &t_args[i]);
    }

    for (int i = 0; i < num_threads; i++) {
        pthread_join(threads[i], NULL);
    }
    double end_tn = get_time();
    double tn = end_tn - start_tn;

    double speedup = t1 / tn;
    double efficiency = speedup / num_threads;

    printf("\nРезультаты эксперимента:\n");
    printf("Потоков:         %d\n", num_threads);
    printf("Время(сек):      %.6f\n", tn);
    printf("Ускорение:       %.2f\n", speedup);
    printf("Эффективность:   %.4f\n", efficiency);
    printf("\nНайдено совпадений: %ld (Проверка: %s)\n", 
           global_match_count, global_match_count == seq_matches ? "OK" : "FAIL");

    pthread_mutex_destroy(&count_mutex);
    pthread_attr_destroy(&attr);
    free(text); free(threads); free(t_args);
    return 0;
}
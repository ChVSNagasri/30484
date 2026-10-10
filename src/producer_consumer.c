
#define _POSIX_C_SOURCE 200809L

#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#include "producer_consumer.h"

#define BUFFER_SIZE 5
#define ITEM_COUNT 10

static int buffer[BUFFER_SIZE];
static int in = 0;
static int out = 0;

static sem_t empty_slots;
static sem_t filled_slots;
static pthread_mutex_t mutex;

static void sleep_ms(long milliseconds)
{
    struct timespec delay;

    delay.tv_sec = milliseconds / 1000;
    delay.tv_nsec = (milliseconds % 1000) * 1000000L;

    nanosleep(&delay, NULL);
}

static void *producer(void *arg)
{
    (void)arg;

    for (int item = 1; item <= ITEM_COUNT; item++) {
        sem_wait(&empty_slots);
        pthread_mutex_lock(&mutex);

        buffer[in] = item;
        printf("Producer produced: %d\n", item);
        in = (in + 1) % BUFFER_SIZE;

        pthread_mutex_unlock(&mutex);
        sem_post(&filled_slots);

        sleep_ms(100);
    }

    return NULL;
}

static void *consumer(void *arg)
{
    (void)arg;

    for (int i = 0; i < ITEM_COUNT; i++) {
        sem_wait(&filled_slots);
        pthread_mutex_lock(&mutex);

        int item = buffer[out];
        printf("Consumer consumed: %d\n", item);
        out = (out + 1) % BUFFER_SIZE;

        pthread_mutex_unlock(&mutex);
        sem_post(&empty_slots);

        sleep_ms(150);
    }

    return NULL;
}

void producer_consumer_command(void)
{
    pthread_t producer_thread;
    pthread_t consumer_thread;

    in = 0;
    out = 0;

    if (sem_init(&empty_slots, 0, BUFFER_SIZE) != 0) {
        perror("sem_init empty_slots");
        return;
    }

    if (sem_init(&filled_slots, 0, 0) != 0) {
        perror("sem_init filled_slots");
        sem_destroy(&empty_slots);
        return;
    }

    if (pthread_mutex_init(&mutex, NULL) != 0) {
        fprintf(stderr, "Failed to initialize mutex\n");
        sem_destroy(&empty_slots);
        sem_destroy(&filled_slots);
        return;
    }

    int producer_created =
        pthread_create(&producer_thread, NULL, producer, NULL) == 0;

    if (!producer_created) {
        fprintf(stderr, "Failed to create producer thread\n");
    }

    int consumer_created =
        pthread_create(&consumer_thread, NULL, consumer, NULL) == 0;

    if (!consumer_created) {
        fprintf(stderr, "Failed to create consumer thread\n");
    }

    if (producer_created && !consumer_created) {
        /*
         * Let the producer finish even if the consumer thread
         * could not be created.
         */
        for (int i = 0; i < ITEM_COUNT; i++) {
            sem_wait(&filled_slots);
            sem_post(&empty_slots);
        }
    }

    if (producer_created) {
        pthread_join(producer_thread, NULL);
    }

    if (consumer_created) {
        pthread_join(consumer_thread, NULL);
    }

    pthread_mutex_destroy(&mutex);
    sem_destroy(&empty_slots);
    sem_destroy(&filled_slots);

    printf("Producer-Consumer demonstration completed.\n");
}


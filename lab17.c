#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

pthread_mutex_t mutex;

struct List {
    char * str;
    pthread_mutex_t mutex;
    struct List * next;
};

struct List * records = NULL;

void freeList(struct List * head) {
    struct List * p = head;
    while (p != NULL) {
        p = p->next;
        free(head);
        head = p;
    }
}

void add(struct List ** head, char * str) {
    struct List * p = (struct List *)calloc(1, sizeof(struct List));
    pthread_mutex_init(&p->mutex, NULL);
    p->str = str;
    p->next = *head;
    *head = p;
}

void printList(struct List * head) {
    struct List * p = head;

    while (p != NULL) {
        pthread_mutex_lock(&p->mutex);
        printf("%s\n", p->str);
        struct List * prev = p;
        p = p->next;
        pthread_mutex_unlock(&prev->mutex);
    }
}

void swap(struct List * x, struct List * y) {
    char * temp = x->str;
    x->str = y->str;
    y->str = temp;
}

void bubbleSort(struct List * head) {
    if (head == NULL) {
        return;
    }
    struct List * p = head;
    struct List * end = NULL;
    char isChange = 0;

    do {
        isChange = 0;
        p = head;

        pthread_mutex_lock(&p->mutex);
        while (p->next != end) {
            pthread_mutex_lock(&p->next->mutex);
            if (strcmp(p->str, p->next->str) > 0) {
                swap(p, p->next);
                isChange = 1;
            }
            struct List * prev = p;
            p = p->next;
            pthread_mutex_unlock(&prev->mutex);
        }
        pthread_mutex_unlock(&p->mutex);
        end = p;
    } while (isChange);
}

void * run(void * param) {
    while (1) {
        sleep(5);
        pthread_mutex_lock(&mutex);
        struct List * arg = records;
        pthread_mutex_unlock(&mutex);
        bubbleSort(arg);
    }
}

int main(int argc, char *argv[]) {
    pthread_mutex_init(&mutex, NULL);

    pthread_t thread, thread1;
    int code = pthread_create(&thread, NULL, run, NULL);
    if (code != 0) {
        perror("pthread_create");
        /*
        1) EAGAIN – The system lacked the necessary resources to create another thread,
            or the system-imposed limit on the total number of threads in a process PTHREAD_THREADS_MAX would be
exceeded
        2) EINVAL – The value specified by attr is invalid
        3) EPERM – The caller does not have appropriate permission to set the required scheduling parameters
or scheduling policy.
        */
    }

    while (1) {
        char * buf = (char *)calloc(80, sizeof(char));
        int rm = read(fileno(stdin), buf, 80);
        if (rm == 1) {
            pthread_mutex_lock(&mutex);
            struct List * arg = records;
            pthread_mutex_unlock(&mutex);
            printList(arg);
        }
        else {
            buf[rm - 1] = '\0';
            if (strcmp("exit", buf) == 0) {
                break;
            }
            pthread_mutex_lock(&mutex);
            add(&records, buf);
            pthread_mutex_unlock(&mutex);
        }
    }

    pthread_cancel(thread);
    if (pthread_join(thread, NULL) != 0) {
        perror("pthread_join");
        /*
        1) EDEADLK
        2) EINVAL - this thread is a detached thread.
        3) ESRCH - NotFoundException
        */
    }
    freeList(records);
    pthread_exit(NULL);
}
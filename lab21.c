#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>

#define PHILO 5
#define DELAY 30000
#define FOOD 50

pthread_mutex_t forks[PHILO];
pthread_mutex_t forks_mutex;
pthread_cond_t forks_cond;
pthread_t phils[PHILO];

void *philosopher (void *id);
int food_on_table ();
void get_forks (int, int, int);
void down_forks (int, int);
pthread_mutex_t foodlock;

int sleep_seconds = 0;

int main (int argn, char **argv)
{
    int i;
    pthread_mutex_init (&foodlock, NULL);
    pthread_mutex_init (&forks_mutex, NULL);
    pthread_cond_init (&forks_cond, NULL);
    for (i = 0; i < PHILO; i++)
        pthread_mutex_init (&forks[i], NULL);
    for (i = 0; i < PHILO; i++)
        pthread_create (&phils[i], NULL, philosopher, (void *)i);
    for (i = 0; i < PHILO; i++)
        pthread_join (phils[i], NULL);
    return 0;
}

void* philosopher (void *num)
{
    int id;
    int i, left_fork, right_fork, f;
  
    id = (int)num;
    printf ("Philosopher %d sitting down to dinner.\n", id);
    right_fork = id;
    left_fork = id + 1;
    
    if (left_fork == PHILO) {
        left_fork = 0;
    }

    while (f = food_on_table ()) {
        printf ("Philosopher %d: get dish %d.\n", id, f);
        get_forks (id, right_fork, left_fork);
        printf ("Philosopher %d: eating.\n", id);
        usleep (DELAY * (FOOD - f + 1));
        down_forks (left_fork, right_fork);
    }
    printf ("Philosopher %d is done eating.\n", id);
    return (NULL);
}

int food_on_table() {
    static int food = FOOD;
    int myfood;
    pthread_mutex_lock (&foodlock);
    if (food > 0) {
        food--;
    }
    myfood = food;
    pthread_mutex_unlock (&foodlock);
    return myfood;
}

void get_forks (int phil, int fork_1, int fork_2) {
    int result;
    pthread_mutex_lock (&forks_mutex);
    do {
        result = pthread_mutex_trylock(&forks[fork_1]);
        if(result==0) {
            result = pthread_mutex_trylock(&forks[fork_2]);
            if(result){
                pthread_mutex_unlock(&forks[fork_1]);
                pthread_cond_wait(&forks_cond, &forks_mutex);
            }
        }
        else 
            pthread_cond_wait(&forks_cond, &forks_mutex);
  } while(result);
  pthread_mutex_unlock(&forks_mutex);
}

void down_forks (int f1, int f2) {
    pthread_mutex_lock(&forks_mutex);
    pthread_mutex_unlock (&forks[f1]);
    pthread_mutex_unlock (&forks[f2]);
    pthread_cond_broadcast(&forks_cond);
    pthread_mutex_unlock(&forks_mutex);
}
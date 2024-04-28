//Dining Philosophers Problem Implementation for Asymmentric and Symmetric Astronomers

//including C libraries
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <time.h>

#define NUM_ASTRONOMERS 10
#define NUM_ASYMMETRIC 3
#define AVG_EAT_TIME 1
#define MAX_WAIT_TIME 2
int ordering[NUM_ASTRONOMERS];  // 0 means symmetric, 1 means asymmetric
sem_t chopsticks[NUM_ASTRONOMERS];
sem_t mutex;

//states of astronomer 
enum {THINKING, HUNGRY, EATING} state[NUM_ASTRONOMERS];

//tests if neighbours of astronomer are not eating and state of current astronomer is hungry
void test(int i){
    if ((state[(i+NUM_ASTRONOMERS-1)%NUM_ASTRONOMERS] != EATING) && (state[i] == HUNGRY) && (state[(i+1)%NUM_ASTRONOMERS] != EATING)){
        //sets state to eating
        state[i] = EATING;
    }
    else{
        //else sets to hungry
        state[i] = HUNGRY;
        printf("Astronomer %d is hungry...\n",i);
    }
}

//think state of astronomer
void think(int i) { 
    
    state[i] = THINKING ; 
    printf("Astronomer %d is thinking...\n", i);
    sleep(MAX_WAIT_TIME);
    
}

//eating state of astronomer
void eat(int i) {

    //if astronomer is asymmetric
    if (ordering[i]==1) {   
        sem_wait(&mutex);

        //waits for right chopstick
        if (sem_wait(&chopsticks[i]) == 0) { 
            //sets state to hungry
            state[i] = HUNGRY; 
            printf("Astronomer %d is hungry..\n",i);
            //calls test to check its neighbours and current state
            test(i);
            printf("Astronomer %d picked up right chopstick.\n",i);
            //acquires left chopstick after MAX_WAIT_TIME
            sleep(MAX_WAIT_TIME); 
            
           
           //waits for left chopstick
            if (sem_wait(&chopsticks[(i + 1) % NUM_ASTRONOMERS]) == 0) { 
                printf("Astronomer %d picked up left chopstick.\n",i);
                //sets state to eating
                state[i] = EATING; 
                //eats for random time
                int random = rand() % AVG_EAT_TIME;
                printf("Astronomer %d is eating...\n", i);
                sleep(random);
                
                //releases both right and left chopstick
                sem_post(&chopsticks[i]); 
                sem_post(&chopsticks[(i + 1) % NUM_ASTRONOMERS]); 
                printf("Astronomer %d put down both chopsticks.\n",i);
            } 

            //runs when astronomer couldn't acquire left chopstick
            else {
                //sets state to hungry
                state[i] = HUNGRY; 
                printf("Astronomer %d is hungry..\n",i);
                //tests for neighbours and current state
                test(i);
                //releases right chopstick also
                sem_post(&chopsticks[i]); 
                printf("Astronomer %d couldn't pick up right chopstick.\n",i);
            }
            
        }
            //sets state to thinking
            state[i] = THINKING; 
            //tests for neighbours
            test((i + 1) % NUM_ASTRONOMERS);
            test((i + NUM_ASTRONOMERS-1) % NUM_ASTRONOMERS);
            think(i);
            sem_post(&mutex);

    }

    //if astronomer is symmetric
    else if(ordering[i]==0) { 
        sem_wait(&mutex);

        //waits for right and left chopstick
        if (sem_wait(&chopsticks[i]) == 0 && sem_wait(&chopsticks[(i + 1) % NUM_ASTRONOMERS]) == 0) {
            //sets state to hungry
            state[i] = HUNGRY; 
            printf("Astronomer %d is hungry...\n",i);
            //tests for neighbour and current state
            test(i);
            printf("Astronomer %d picked up both chopsticks.\n",i);
            //sets state to eating after picking up chopsticks
            state[i] = EATING; 
            //eats for random time
            int random = rand() % AVG_EAT_TIME;
            printf("Astronomer %d is eating...\n", i);
            sleep(random);
            //releases both right and left chopstick
            sem_post(&chopsticks[i]); 
            sem_post(&chopsticks[(i + 1) % NUM_ASTRONOMERS]);
            printf("Astronomer %d put down both chopsticks.\n",i);
            
        }
            //sets state to thining
            state[i] = THINKING; 
            //test for neighbours
            test((i + 1) % NUM_ASTRONOMERS);
            test((i + NUM_ASTRONOMERS-1) % NUM_ASTRONOMERS);
            think(i);
            sem_post(&mutex);
    
     
    }
}

//controls state of astronomer in infinite while loop
void* philosopher(void* arg) {
    int *i = arg;
    while (1) {
        think(*i);
        eat(*i);
    }
}

//orders the astronomers position
void place_astronomers(int *ordering) {
    for (int i = 0; i < NUM_ASYMMETRIC; i++) {
        ordering[i] = 1; // Asymmetric
    }
    for (int i = NUM_ASYMMETRIC; i < NUM_ASTRONOMERS; i++) {
        ordering[i] = 0; // Symmetric
    }
    for (int i = 0; i < NUM_ASTRONOMERS; i++) {
        int j = rand() % NUM_ASTRONOMERS;
        int temp = ordering[i];
        ordering[i] = ordering[j];
        ordering[j] = temp;
    }
}

int main() {

    //intializes threads
    pthread_t philosophers[NUM_ASTRONOMERS];

    place_astronomers(ordering);

    //starting state of astronomer
    for(int i=0; i<NUM_ASTRONOMERS;i++){
        state[i] = THINKING; 
    }
    
    //initializes semaphores for chopsticks
    for (int i = 0; i < NUM_ASTRONOMERS; i++) {
        sem_init(&chopsticks[i], 0, 1);
    }

    //initializes mutex
    sem_init(&mutex, 0, 1);
   
    //create threads
    for (int i = 0; i < NUM_ASTRONOMERS; i++) {
        int *philosopher_id = (int *)malloc(sizeof(int));
        *philosopher_id = i;
        pthread_create(&philosophers[i], NULL, philosopher, (void *)philosopher_id);
    }

    //join threads
    for (int i = 0; i < NUM_ASTRONOMERS; i++) {
        pthread_join(philosophers[i], NULL);
    }

   //destroys semaphores for chopsticks
    for (int i = 0; i < NUM_ASTRONOMERS; i++) {
        sem_destroy(&chopsticks[i]);
    }

    //destroys mutex
    sem_destroy(&mutex);
    
    return 0;
}

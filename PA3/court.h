#ifndef PA3_COURT_H
#define PA3_COURT_H

#include <pthread.h>
#include <iostream>
#include <vector>
#include <unistd.h>
#include <semaphore.h>
#include <stdexcept>
#include <cstdio>

using namespace std;

class Court { // my main Court class implementation

private:
    int neededPlayers; // how many players we need to play a game
    int hasReferee; // check whether there is a referee or not, 0 or 1
    int playerCount = 0; // increases when threads come
    bool MIP = false; // my bool to check if a match is in progress
    pthread_mutex_t lock{}; // a mutex lock
    pthread_t refereeTID; // to assign a referee
    bool refereeCheck = true; // to check if the referee has left

    // for this homework, I implemented 3 semaphores
    sem_t accessToCourt{};
    sem_t refereeFirst{};
    sem_t matchOver{};

public:
    Court(int players, int referee);
    void play(); // not implemented
    void enter();
    void leave();
};

Court::Court(int players, int referee) { // main constructor
    if (players <= 0) { // checks if player count is a positive number
        throw invalid_argument("An error occurred");
    }
    if (referee != 0 && referee != 1) { // checks if referee count is between 0 and 1
        throw invalid_argument("An error occurred");
    }
    neededPlayers = players;
    hasReferee = referee;
    pthread_mutex_init(&lock, NULL); // initializing the mutex lock
    sem_init(&accessToCourt, 0, 0);
    sem_init(&refereeFirst, 0, 0);
    sem_init(&matchOver, 0, 0);
}

void Court::enter() { // my enter method
    pthread_t tid = pthread_self(); // getting the thread ID

    pthread_mutex_lock(&lock);
    printf("Thread ID: %ld, I have arrived at the court.\n", (long)tid);

    while (MIP) { // while a match is in progress
        pthread_mutex_unlock(&lock);
        sem_wait(&accessToCourt);
        pthread_mutex_lock(&lock);
    }

    playerCount++;
    if ((!hasReferee && playerCount == neededPlayers) || (hasReferee && playerCount == neededPlayers + 1)) { // case of match starting
        MIP = true;
        printf("Thread ID: %ld, There are enough players, starting a match.\n", (long)tid);

        if (hasReferee) { // if has a referee
            refereeTID = tid;
            refereeCheck = false;
        }
    } else {
        printf("Thread ID: %ld, There are only %d players, passing some time.\n", (long)tid, playerCount);
    }

    pthread_mutex_unlock(&lock);
}

void Court::leave() { // my leave method
    pthread_t tid = pthread_self();

    pthread_mutex_lock(&lock);

    if (MIP) { // if a match is in progress
        if (hasReferee && pthread_equal(tid, refereeTID)) { // if the thread is the referee
            printf("Thread ID: %ld, I am the referee and now, match is over. I am leaving.\n", (long)tid);
            for (int i = 0; i < neededPlayers; i++) {
                sem_post(&refereeFirst); // signal players to leave
            }
            refereeCheck = true;
            sem_post(&matchOver);
        } else {
            if (!refereeCheck && hasReferee) { // if the referee is still in the game
                pthread_mutex_unlock(&lock);
                sem_wait(&refereeFirst);
                pthread_mutex_lock(&lock);
                printf("Thread ID: %ld, I am a player and now, I am leaving.\n", (long)tid);
            } else {
                printf("Thread ID: %ld, I am a player and now, I am leaving.\n", (long)tid);
            }
        }
    } else {
        printf("Thread ID: %ld, I was not able to find a match and I have to leave.\n", (long)tid);
    }

    playerCount--; // decrease for every thread that left

    if (playerCount == 0 && MIP) { // if a game has ended
        MIP = false;
        for (int i = 0; i < neededPlayers + (hasReferee ? 1 : 0); ++i) {
            sem_post(&accessToCourt);
        }
        printf("Thread ID: %ld, everybody left, letting any waiting people know.\n", (long)tid);
        if (hasReferee) {
            sem_post(&matchOver);
        }
    }

    pthread_mutex_unlock(&lock);
}

#endif // PA3_COURT_H

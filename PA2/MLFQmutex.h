//
// Created by Berke Ayyildizli 31018
//

#ifndef PA2_MLFQMUTEX_H
#define PA2_MLFQMUTEX_H

#include <pthread.h> //for the threads
#include <vector> //for holding the queues
#include <atomic> //for the bool
#include <chrono>// for the times
#include <iostream>//for the i/o
#include <unordered_map>//for holding the priority level info
#include "queue.h"//my queue class

using namespace std;

class MLFQMutex{

private:

    double qval; //quantum value
    double execTime; //execution time
    int maximumLevel; //maximum level of queues

    pthread_mutex_t mutexLock; //the mutex lock

    atomic<bool> locked; //atomic boolean
    chrono::steady_clock::time_point timeOfStart; //start time
    chrono::steady_clock::time_point timeOfEnd; //end time

    vector<Queue<pthread_t>> prioQueues;// vector of queues of threads

    unordered_map<pthread_t, int> prioThread; //hash map of threads and their respected priorities

    pthread_t owner; //holds the info of which thread holds the lock

public:

    MLFQMutex(int levels, double quantumValue){ //constructor

        maximumLevel = levels;
        qval = quantumValue;
        execTime = 0;
        pthread_mutex_init(&mutexLock,NULL);
        locked = false;
        prioQueues.resize(maximumLevel);  // creates a queue for each level

    }

    void lock() { //main lock method

        pthread_t idOfThread = pthread_self(); //gets the id of current thread
        pthread_mutex_lock(&mutexLock);

        if (prioThread.count(idOfThread) == 0) { //this block checks if the current thread is on the hashmap, if not makes its priority 0
            prioThread[idOfThread] = 0;
            cout<< "Adding thread with ID: " << idOfThread << " to level 0" <<endl; //adds to priority 0 queue
            prioQueues[0].enqueue(owner); //adds to priority 0 queue
        }

        int priority = prioThread[idOfThread]; //if on the hashmap, just makes it access the value
        pthread_mutex_unlock(&mutexLock);

        while (true) { // if did not acquire the lock, thread comes here and spins
            bool expected = false; //for the atomic function to work

            if (locked.compare_exchange_strong(expected, true)) { //this method does a compare and swap operation
                pthread_mutex_lock(&mutexLock);
                owner = idOfThread; //owner is now the current thread
                timeOfStart = chrono::steady_clock::now(); //starts the timer
                pthread_mutex_unlock(&mutexLock);
                return;
            }
            else {
                sched_yield(); //before trying, yields back to cpu
            }
        }
    }

    void unlock() { //the main unlock method
        pthread_mutex_lock(&mutexLock);

        if (pthread_self() != owner) { //if the accessing thread is not the owner of the lock, gives an error
            pthread_mutex_unlock(&mutexLock);
            throw runtime_error("WRONG OWNER!");
        }

        timeOfEnd = chrono::steady_clock::now(); // stopping the timer
        auto execTime = chrono::duration_cast<chrono::seconds>(timeOfEnd - timeOfStart).count(); //counting as seconds
        int currentPriority = prioThread[owner]; //gets the current priority of the thread
        int newPriority = currentPriority;

        if (execTime > qval) { //checks if it is more than quantum value
            int priorityIncrease = execTime / qval; //calculates how many levels to increase
            newPriority = min(currentPriority + priorityIncrease, maximumLevel - 1);
        }

        if (newPriority != currentPriority) { //if there is a priority change
            cout << "Adding thread with ID: " << owner << " to level " << newPriority << endl;

            prioQueues[currentPriority].dequeue(); //dequeues from the old level
            prioQueues[newPriority].enqueue(owner); //enqueues to the new level
            prioThread[owner] = newPriority; //priority is updated

        }

        owner = pthread_t(); //resetting the owner
        locked.store(false); // unlocking the bool

        pthread_mutex_unlock(&mutexLock);
    }

    void print(){

        pthread_mutex_lock(&mutexLock);
        cout<<"Waiting treads :"<<endl;

        for (int i = 0; i < maximumLevel; ++i) {
            cout<<"Level " <<i<<": ";
            prioQueues[i].print();
        }

        pthread_mutex_unlock(&mutexLock);

    }

};

#endif //PA2_MLFQMUTEX_H

#ifndef PA2_QUEUE_H
#define PA2_QUEUE_H
//created by Berke Ayyildizli 31018
#include <iostream>
#include <pthread.h>
#include <stdexcept> //for the exceptions

using namespace std;

template <typename T>
struct QueueNodes { //a struct for the nodes
    T information;
    QueueNodes* next;

    QueueNodes(T node){ //constructor
        information = node;
        next = NULL;
    }

};

template <typename T> //a templated queue
class Queue {
private:
    QueueNodes<T>* head;
    QueueNodes<T>* tail;
    pthread_mutex_t HeadMutex; //2 locks for head and tail of queue
    pthread_mutex_t TailMutex;

public:
    Queue(){
        head = NULL;
        tail = NULL;
        pthread_mutex_init(&HeadMutex, NULL);
        pthread_mutex_init(&TailMutex, NULL);
    }

    ~Queue() {
        while (!isEmpty()) {
            dequeue();
        }
        pthread_mutex_destroy(&HeadMutex);
        pthread_mutex_destroy(&TailMutex);
    }

    void enqueue(T item) { //enqueue method, got help from the michael and scott queue
        pthread_mutex_lock(&TailMutex); //when enqueing, locking the tail lock
        QueueNodes<T>* newNode = new QueueNodes<T>(item); //a dummy node

        if (tail == NULL) {
            head = newNode;
            tail = newNode;
        }
        else {
            tail->next = newNode;
            tail = newNode;
        }
        pthread_mutex_unlock(&TailMutex);
    }

    T dequeue() { //returns a template object
        pthread_mutex_lock(&HeadMutex);
        if (head == NULL) { // if the queue is empty, gives an exception
            pthread_mutex_unlock(&HeadMutex);
            throw runtime_error("THE QUEUE IS EMPTY!");
        }

        QueueNodes<T>* temp = head; //another temporary node for holding the to be dequeued node
        head = head->next;
        if (head == NULL) {
            tail = NULL;
        }

        T dequeuedValue = temp->information;
        delete temp;
        pthread_mutex_unlock(&HeadMutex);
        return dequeuedValue;
    }

    bool isEmpty() { //checks if the head is null
        pthread_mutex_lock(&HeadMutex);
        bool result = (head == NULL);
        pthread_mutex_unlock(&HeadMutex);
        return result;
    }

    void print() { //just iterates through all the queue and prints the whole information across nodes

        pthread_mutex_lock(&HeadMutex);
        QueueNodes<T>* current = head;
        if (current == NULL){
            cout<<"Empty";
        }
        while (current != NULL) {
            cout << current->information << " ";
            current = current->next;
        }
        cout << endl;
        pthread_mutex_unlock(&HeadMutex);

    }
};

#endif //PA2_QUEUE_H

#include <iostream>
#include <stdio.h>
#include <pthread.h>
#include "monitor.h"

#define BUFFER_SIZE 8
#define MAX_PRODUCTION_SIZE 3
#define ALPHABET_LENGTH 26

struct Element {
  char label;
  unsigned int lastReadBy;
};


class PCMonitor:Monitor {
  Element buffer[BUFFER_SIZE];
  int count, head, tail;
  int size1, size2;
  Condition producer1, producer2;
  Condition consumer1, consumer2, consumer3;

public:
  PCMonitor() {
    this->count = 0;
    this->head = 0;
    this->tail = 0;
    this->size1 = 0;
    this->size2 = 0;
  }

  void producer1Task() {
    while(1) {
      enter();
        size1 = rand() % MAX_PRODUCTION_SIZE;
        size1 += 1; //Avoid 0 size
      printf("Producer1 wants to insert: %d elements, free space: %d\n",
        size1, BUFFER_SIZE - count);

      while(BUFFER_SIZE - count < size1) wait(producer1);

      for(int i = 0; i < size1; ++i) {
        buffer[tail].label = 'a' + (rand() % ALPHABET_LENGTH);
        buffer[tail].lastReadBy = 0;
        ++count;

        printf("Producer1 inserts:\t%c\t[%d]\n", buffer[tail].label, count);

        tail = (tail + 1) % BUFFER_SIZE;
      }

      if(count - size1 == 0) signal(consumer1);
        size1 = 0;
      if(count > 4) signal(consumer3);

      leave();
      sleep(1);
    }
  }

  void producer2Task() {
    while(1) {
      enter();
        size2 = rand() % MAX_PRODUCTION_SIZE;
        size2 += 1; //Avoid 0 size
        printf("Producer2 wants to insert: %d elements, free space: %d\n",
          size2, BUFFER_SIZE - count);

      while(BUFFER_SIZE - count < size2) wait(producer2);

      for(int i = 0; i < size2; ++i) {
        buffer[tail].label = 'a' + (rand() % ALPHABET_LENGTH);
        buffer[tail].lastReadBy = 0;
        ++count;

        printf("Producer2 inserts:\t%c\t[%d]\n", buffer[tail].label, count);

        tail = (tail + 1) % BUFFER_SIZE;
      }

      if(count - size2 == 0) signal(consumer1);
        size1 = 0;
      if(count > 4) signal(consumer3);

      leave();
      sleep(1);
    }
  }

  void consumer1Task() {
    while(1) {
      enter();
      int id = 1;

      while(count < 1 || (buffer[head].lastReadBy != (id-1)
          && buffer[head].lastReadBy != id)) wait(consumer1);

      printf("Consumer%d:\tread: %c\t[%d]\n", id, buffer[head].label, count);
      buffer[head].lastReadBy = id;

      signal(consumer2);
      leave();
      sleep(1);
    }
  }

  void consumer2Task() {
    while(1) {
      enter();
      int id = 2;

      while(count < 1 || (buffer[head].lastReadBy != (id-1)
          && buffer[head].lastReadBy != id)) wait(consumer2);

      printf("Consumer%d:\tread: %c\t[%d]\n", id, buffer[head].label, count);
      buffer[head].lastReadBy = id;

      signal(consumer3);
      leave();
      sleep(1);
    }
  }

  void consumer3Task() {
    while(1) {
      enter();
      int id = 3;

      while(count < 4 || buffer[head].lastReadBy != (id-1)) wait(consumer3);

      --count;
      printf("Consumer%d:\tread and remove: %c\t[%d]\n", id,
        buffer[head].label, count);
      head = (head + 1) % BUFFER_SIZE;

      if(count > 0) signal(consumer1);
      if(size1 > 0 && BUFFER_SIZE - count >= size1) signal(producer1);
      if(size2 > 0 && BUFFER_SIZE - count >= size2) signal(producer2);

      leave();
      sleep(1);
    }
  }

};

PCMonitor monitor;

void *producer1Task(void* p) { monitor.producer1Task();}
void *producer2Task(void* p) { monitor.producer2Task();}
void *consumer1Task(void* p) { monitor.consumer1Task();}
void *consumer2Task(void* p) { monitor.consumer2Task();}
void *consumer3Task(void* p) { monitor.consumer3Task();}

int main() {
  pthread_t prod1, prod2, cons1, cons2, cons3;
  int error = 0;

  error = pthread_create(&prod1, NULL, producer1Task, NULL);
  if(error) printf("Cannot create new thread properly\n");

  error = pthread_create(&prod2, NULL, producer2Task, NULL);
  if(error) printf("Cannot create new thread properly\n");

  error = pthread_create(&cons1, NULL, consumer1Task, NULL);
  if(error) printf("Cannot create new thread properly\n");

  error = pthread_create(&cons2, NULL, consumer2Task, NULL);
  if(error) printf("Cannot create new thread properly\n");

  error = pthread_create(&cons3, NULL, consumer3Task, NULL);
  if(error) printf("Cannot create new thread properly\n");

  pthread_join(prod1, NULL);
  pthread_join(prod2, NULL);
  pthread_join(cons1, NULL);
  pthread_join(cons2, NULL);
  pthread_join(cons3, NULL);

  return 0;
}

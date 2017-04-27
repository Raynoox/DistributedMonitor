//
// Created by root on 4/13/17.
//
#include "monitor.h"

#ifndef DISTRIBUTEDMONITOR_CONSUMER_H
#define DISTRIBUTEDMONITOR_CONSUMER_H
class Consumer : public Monitor {
public:
    Consumer(zmq::context_t &ctx, int arraySize, int procId, int procNum)
            : Monitor(ctx, arraySize, procId, procNum) {}

    void consume() {
        printMessage("trylock");
        lock(1);
        cout << consMod << " " << size << endl;
        while (dataArray->getValue()[consMod % size] == 0) {
            cout << "waiting" << endl;
            wait(2, 1);
            //pthread_cond_wait(&c2, &m);
        }
        dataArray->getValue()[consMod % size] = 0;
        printf("%d CONSUMING VALUE AT %d\n", pid, consMod % size);
        consMod++;
        signal(1);
        //pthread_cond_signal(&c);
        unlock(1);
        //pthread_mutex_unlock(&m);

    }
};
#endif //DISTRIBUTEDMONITOR_CONSUMER_H

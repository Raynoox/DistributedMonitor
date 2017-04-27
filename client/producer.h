//
// Created by root on 4/13/17.
//
#include "monitor.h"
#ifndef DISTRIBUTEDMONITOR_PRODUCER_H
#define DISTRIBUTEDMONITOR_PRODUCER_H
class Producer : public Monitor {
public:
    Producer(zmq::context_t &ctx, int arraySize, int procId, int procNum)
            : Monitor( ctx, arraySize, procId, procNum){}
    void produce() {
        cout<<"p"<<endl;
        lock(1);
        printArray();
        cout<<"CHECK DATA -> modulo = "<<prodMod<<endl;
        while(!(dataArray->getValue()[prodMod%size] == 0)) {
            wait(1,1);
        }
        dataArray->getValue()[prodMod%size] = 1;
        printf("%d PRODUCING VALUE AT %d\n",pid, prodMod % size);
        prodMod++;
        signal(2);
        unlock(1);
    }

};
#endif //DISTRIBUTEDMONITOR_PRODUCER_H

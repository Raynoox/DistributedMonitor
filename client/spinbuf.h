//
// Created by root on 4/13/17.
//
#include "monitor.h"
#ifndef DISTRIBUTEDMONITOR_SPINBUF_H
#define DISTRIBUTEDMONITOR_SPINBUF_H
class Spinbuf : public Monitor {
public:
    Spinbuf(zmq::context_t &ctx, int arraySize, int procId, int procNum, int mon_id)
            : Monitor( ctx, arraySize, procId, procNum, 0, mon_id){}
    int* getBuffer() {
        lock(1);
        unlock(1);
        return dataArray->getValue();
    }
    void putInto(int position,int value) {
        lock(1);
        while(!(dataArray->getValue()[position] == 0)) {
            wait(1,1);
        }
        dataArray->getValue()[position] = value;
        printf("%d PRODUCING VALUE %d AT %d\n",pid, value, position);
        //prodMod++;
        signal(2);
        unlock(1);
    }
    int getAndClear(int position) {
        lock(1);
        while (dataArray->getValue()[position] == 0) {
            wait(2, 1);
        }
        int value = dataArray->getValue()[position];
        dataArray->getValue()[position] = 0;
        printf("%d CONSUMING VALUE %d AT %d\n", pid, value, position);
        //consMod++;
        signal(1);
        unlock(1);
        return value;

    }

};
#endif //DISTRIBUTEDMONITOR_SPINBUF_H

//
// Created by root on 4/13/17.
//
#include "monitor.h"
#include "lockwrapper.h"
#ifndef DISTRIBUTEDMONITOR_SPINBUF_H
#define DISTRIBUTEDMONITOR_SPINBUF_H
class Spinbuf : public Monitor {
public:
    Spinbuf(zmq::context_t &ctx, int arraySize, int procId, int procNum, int mon_id)
            : Monitor( ctx, arraySize, procId, procNum, 0, mon_id){}
    int* getBuffer() {
        Lockwrapper l(this);
        return dataArray->getValue();
    }
    void putInto(int position,int value) {
        Lockwrapper l(this);
        while(!(dataArray->getValue()[position] == 0)) {
            wait(1);
        }
        dataArray->getValue()[position] = value;
        printf("%d PRODUCING VALUE %d AT %d\n",pid, value, position);
        signal(2);
    }
    void putIntoNext(int value) {
        Lockwrapper l(this);
        while(!(dataArray->getValue()[prodMod%size] == 0)) {
            wait(1);
        }
        dataArray->getValue()[prodMod%size] = value;
        printf("%d PRODUCING VALUE %d AT %d\n",pid, value, prodMod%size);
        prodMod++;
        signal(2);
    }
    int getAndClear(int position) {
        Lockwrapper l(this);
        while (dataArray->getValue()[position] == 0) {
            wait(2);
        }
        int value = dataArray->getValue()[position];
        dataArray->getValue()[position] = 0;
        consMod++;
        printf("%d CONSUMING VALUE %d AT %d\n", pid, value, position);
        signal(1);
        return value;
    }
    int getAndClearNext() {
        Lockwrapper l(this);
        while (dataArray->getValue()[consMod%size] == 0) {
            wait(2);
        }
        int value = dataArray->getValue()[consMod%size];
        dataArray->getValue()[consMod%size] = 0;
        printf("%d CONSUMING VALUE %d AT %d\n", pid, value, consMod%size);
        consMod++;
        signal(1);
        return value;
    }
    int getProducerModulo() {
        Lockwrapper l(this);
        return prodMod;
    }
    void incrementProducerModulo() {
        Lockwrapper l(this);
        prodMod++;
    }
    int getConsumerModulo() {
        Lockwrapper l(this);
        return consMod;
    }
    void incrementConsumerModulo() {
        Lockwrapper l(this);
        consMod++;
    }

};
#endif //DISTRIBUTEDMONITOR_SPINBUF_H

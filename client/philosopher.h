//
// Created by root on 5/3/17.
//


//
// Created by root on 4/13/17.
//
#include "monitor.h"
#ifndef DISTRIBUTEDMONITOR_PHILOSOPHER_H
#define DISTRIBUTEDMONITOR_PHILOSOPHER_H
class Philosopher : public Monitor {
public:
    Philosopher(zmq::context_t &ctx, int arraySize, int procId, int procNum)
            : Monitor( ctx, arraySize, procId, procNum, 2){
            proc_id = procId;
    }
    void eat() {
        cout<<"p"<<endl;
        lock(1);
        printArray();
        cout<<"CHECK DATA -> modulo = "<<prodMod<<endl;
//        while(!((dataArray->getValue()[(proc_id-1)%size] == 2) && (dataArray->getValue()[(proc_id+1)%size] == 2))) {
        while(!((dataArray->getValue()[(proc_id)%size] == 2))) {
            wait(1,1);
        }
        cout<<"EATING"<<endl;
        dataArray->getValue()[(proc_id-1)%size]--;
        dataArray->getValue()[(proc_id+1)%size]--;
        unlock(1);
        sleep(1);
        cout<<"WAKING, WAITING FOR LOCK"<<endl;
        lock(1);
        dataArray->getValue()[(proc_id-1)%size]++;
        dataArray->getValue()[(proc_id+1)%size]++;
        cout<<"UNLOCKING"<<endl;
        signal(1);
        unlock(1);
    }
private:
    int proc_id;
};
#endif //DISTRIBUTEDMONITOR_PHILOSOPHER_H



//
// Created by root on 5/17/17.
//
#include "monitor.h"
#include "lockwrapper.h"
#ifndef DISTRIBUTEDMONITOR_TABLE_H
#define DISTRIBUTEDMONITOR_TABLE_H
class Table : public Monitor {
public:
    Table(zmq::context_t &ctx, int arraySize, int procId, int procNum, int mon_id)
            : Monitor( ctx, arraySize, procId, procNum, 2, mon_id){
        proc_id = procId;
        monitor_id = mon_id;
    }
    void pickForks(int place) {
        Lockwrapper l(this);
        while(!((dataArray->getValue()[(place)%size] == 2))) {
            wait(1);
        }
        //negative modulo hack
        if(place == 0){
            dataArray->getValue()[(size-1)]--;
        } else {
            dataArray->getValue()[(place-1)%size]--;
        }
        dataArray->getValue()[(place+1)%size]--;
        dataArray->print();
    }
    void releaseForks(int place) {
        Lockwrapper l(this);
        cout<<"RELEASING FORKS"<<endl;
        if(place == 0){
            dataArray->getValue()[(size-1)]++;
        } else {
            dataArray->getValue()[(place-1)%size]++;
        }
        dataArray->getValue()[(place+1)%size]++;
        signal(1);
    }
private:
    int monitor_id;
    int proc_id;
};
#endif //DISTRIBUTEDMONITOR_TABLE_H

//
// Created by root on 4/1/17.
//
#include <sstream>
#include <queue>
#include <utility>
#include <zmq.hpp>
#ifndef DISTRIBUTEDMONITOR_MONITOR_H
#define DISTRIBUTEDMONITOR_MONITOR_H

#endif //DISTRIBUTEDMONITOR_MONITOR_H
using namespace std;
template <class T>
class GenericMessage {
public:

    GenericMessage(int id, T msg):
            beId(id),
            data(msg)
    {}

    ~GenericMessage(){}

    T getData()
    {
        //LINE 18 is the following line!
        return data;
    }

    std::string toString()
    {
        std::ostringstream ss;
        ss << getBeId();
        std::string ret = ss.str();
        return ss.str();
    }

    void setBeId(int id)
    {
        beId = id;
    }

    int getBeId()
    {
        return beId;
    }
private:
    int beId;
    T data;
};

class CompareQueue
{
public:
    bool operator()(pair<int,int> n1,pair<int,int> n2) {
        return n1.second == n2.second ? n1.first < n2.first : n1.second < n2.second;
    }
};

//template <class T>
class Monitor {
public:
    Monitor (zmq::context_t &ctx, int arraySize, int procId)
            :        ctx_(ctx),
                     sock_req_(ctx_, ZMQ_DEALER),
                     sock_sub_(ctx_, ZMQ_SUB) {
        value = new int[arraySize]();
        prodMod = 0;
        consMod = 0;
        pid = procId;
        size = arraySize;
//        sock_req_.setsockopt(ZMQ_IDENTITY, pid, sizeof(pid));
        sock_req_.connect("tcp://localhost:5570");
        sock_sub_.connect("tcp://localhost:5571");

//        sock_sub_.setsockopt(ZMQ_IDENTITY, pid, sizeof(pid));
//        const char *filter = (argc > 1)? argv [1]: "ACK_SUB";
//        sock_sub_.setsockopt(ZMQ_SUBSCRIBER, filter, strlen(filter));

    }

//     Monitor (T init){
//        value = first;
//     }
//
//    T get()
//    {
//        return value;
//    }
    void consume() {
        lock();
        //pthread_mutex_lock(&m);
        while(value[consMod%size] == 0) {
            wait();
        //pthread_cond_wait(&c2, &m);
        }
        value[consMod%size] = 0;
        consMod++;
        printf("%d CONSUMING VALUE AT %d\n",pid, consMod);
        signal();
        //pthread_cond_signal(&c);
        unlock();
        //pthread_mutex_unlock(&m);

    }
    void lock() {//mutexId
        //request for mutex
        //wait for ack
        //if got all ack, wait for signal/unlocks till first in queue
    }
    void wait() {
        //unlock mutex
    //wait for signal
        //lock()
        //if got signal, wait for unlocks/signals till first in queue
    }
    void signal() { //with condId, mutexId, values
        //port 5570 request with value, consMod, prodMod, dont need to wait
    }
    void unlock() { //with mutexId, values
        //port 5570 request with value, consMod, prodMod, dont need to wait
    }
private:
    int *value;
    int pid;
//    T value;
    int size;
    int prodMod;
    int consMod;
    zmq::context_t &ctx_;
    zmq::socket_t sock_req_;
    zmq::socket_t sock_sub_;
    priority_queue<pair<int,int>,vector<pair<int,int>>,CompareQueue> pq;
};
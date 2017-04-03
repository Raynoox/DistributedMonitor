//
// Created by root on 4/1/17.
//
#include <sstream>
#include <queue>
#include <utility>
#include <zmq.hpp>
#ifndef DISTRIBUTEDMONITOR_MONITOR_H
#define DISTRIBUTEDMONITOR_MONITOR_H

#define MSG_TYPE_ACK 0x1
#define MSG_TYPE_REQUEST 0x2
#define MSG_TYPE_RELEASE 0x3
#define MSG_TYPE_SIGNAL 0x4

#define PROTOCOL "tcp"

#define LOCAL_REQUEST_PORT "5570"
#define LOCAL_PUB_PORT "5571"

#define LOCAL_REQUEST_URL PROTOCOL "://localhost" LOCAL_REQUEST_PORT
#define LOCAL_PUB_URL PROTOCOL "://localhost" LOCAL_PUB_PORT

using namespace std;
struct Message {
    int pid;
    int time;
    int type;
    int mutexId;
    int condId;
    int data[100];
};
template <class T>
class GenericMessage {
public:

    GenericMessage(int id,int type, T msg):
            beId(id),
            msg_type(type),
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

    void setMsgType(int type)
    {
        msg_type = type;
    }

    int getMsgType()
    {
        return msg_type;
    }
private:
    int beId;
    int msg_type;
    T data;
};

class CompareQueue
{
public:
    bool operator()(pair<int,int> n1,pair<int,int> n2) {
        return n1.second == n2.second ? n1.first < n2.first : n1.second < n2.second;
        //second -> time; first -> pid
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
        ltime = 0;
        pid = procId;
        size = arraySize;
//        sock_req_.setsockopt(ZMQ_IDENTITY, pid, sizeof(pid));
        sock_req_.connect(LOCAL_REQUEST_URL);
        sock_sub_.connect(LOCAL_PUB_URL);

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
        lock(1);
        //pthread_mutex_lock(&m);
        while(value[consMod%size] == 0) {
            wait(2,1);
            //pthread_cond_wait(&c2, &m);
        }
        value[consMod%size] = 0;
        consMod++;
        printf("%d CONSUMING VALUE AT %d\n",pid, consMod);
        signal(1);
        //pthread_cond_signal(&c);
        unlock(1);
        //pthread_mutex_unlock(&m);

    }
    void produce() {
        lock(1);
        while(value[prodMod%size] == 0) {
            wait(1,1);
        }
        value[prodMod%size] = 0;
        prodMod++;
        printf("%d PRODUCING VALUE AT %d\n",pid, prodMod);
        signal(2);
        unlock(1);
    }
    GenericMessage<pair<int,int>> prepare_message(int MSG_TYPE) {
        pair<int,int> p = make_pair(pid,ltime);
        GenericMessage<pair<int,int>> message = GenericMessage<pair<int,int>>(pid, MSG_TYPE, p);
        return message;
    }
    zmq::message_t prepare_empty(int TYPE,int mutexId){
        struct Message m;
        m.pid = pid;
        m.type = TYPE;
        m.mutexId = mutexId;
        m.time = ltime++; //++?
        zmq::message_t msg (sizeof(struct Message));
        memcpy (msg.data (), &m, (sizeof(struct Message)));
        return msg;
    }
    zmq::message_t prepare_message(int TYPE, int mutexId, int condId) {
        struct Message m;
        m.pid = pid;
        m.type = TYPE;
        m.mutexId = mutexId;
        m.condId = condId;
        m.time = ltime++; //++?
        memcpy(m.data, value, size*sizeof(int));
        zmq::message_t msg (sizeof(struct Message));
        memcpy (msg.data (), &m, (sizeof(struct Message)));
        return msg;
    }
    void lock(int mutexId) {
        //request for mutex
        sock_req_.send(prepare_empty(MSG_TYPE_REQUEST,mutexId));
        //wait for ack
        memset(ack, 0, PROC_NUM*sizeof(*ack)); //zero ack table
        while(isWaiting()) {
            //wait for signal GOT_ACK_MESSAGE and check again
        }

        //if got all ack, wait for ?signal?/unlocks till first in queue
        while(pq.top().first != pid) {
            //wait for signal GOT_RELEASE_MESSAGE and check again
        }
    }
    void wait(int condId, int mutexId) {
        //unlock mutex
        unlock(mutexId);
        //wait for signal
        //pthread_wait
        //if got signal, wait for unlocks/signals till first in queue
        lock(mutexId);
    }
    void signal(int condId) {
        sock_req_.send(prepare_message(MSG_TYPE_SIGNAL,-1,condId));
        //with condId, mutexId, values
        //port 5570 request with value, consMod, prodMod, dont need to wait
    }
    void unlock(int mutexId) {
        pq.pop();
        sock_req_.send(prepare_message(MSG_TYPE_RELEASE,mutexId, -1));
        //with mutexId, values
        //port 5570 request with value, consMod, prodMod, dont need to wait
    }
    void handle_msg() {
        //pthread process must loop this on sock_sub_.recv
        //and switch on type
    }
private:
    int *value;
    int *ack;
    int pid;
    int ltime;
    int PROC_NUM;
//    T value;
    int size;
    int prodMod;
    int consMod;
    zmq::context_t &ctx_;
    zmq::socket_t sock_req_;
    zmq::socket_t sock_sub_;
    priority_queue<pair<int,int>,vector<pair<int,int>>,CompareQueue> pq;
    bool isWaiting() {
        for(int i =0;i<PROC_NUM;i++) {
            if(ack[i] == 0) {
                return false;
            }
        }
        return true;
    }
};
#endif //DISTRIBUTEDMONITOR_MONITOR_H

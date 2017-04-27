//
// Created by root on 4/1/17.
//
#include <sstream>
#include <string>
#include <iostream>
#include <queue>
#include <utility>
#include <zmq.hpp>
#include <zhelpers.hpp>
#include <pthread.h>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#ifndef DISTRIBUTEDMONITOR_MONITOR_H
#define DISTRIBUTEDMONITOR_MONITOR_H

#define MSG_TYPE_ACK 0x1
#define MSG_TYPE_REQUEST 0x2
#define MSG_TYPE_RELEASE 0x3
#define MSG_TYPE_SIGNAL 0x4


#define PROTOCOL "tcp"

#define LOCAL_REQUEST_PORT "5570"
#define LOCAL_PUB_PORT "5571"

#define LOCAL_REQUEST_URL PROTOCOL "://localhost:" LOCAL_REQUEST_PORT
#define LOCAL_PUB_URL PROTOCOL "://localhost:" LOCAL_PUB_PORT

using namespace std;
enum MSG_T {ACK, REQUEST, RELEASE, SIGNAL};
static const char * MSG_TStrings[] = { "ACK", "REQUEST", "RELEASE", "SIGNAL" };

struct Message {
    int pid;
    int destpid;
    int time;
    int prodMod;
    int consMod;
    MSG_T type;
    int mutexId;
    int condId;
    int data[100];
    char dataString[1024];
};
class DataSerial {
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, unsigned int version)
    {
        int i;
        for(i = 0; i < size; ++i)
            ar & value[i];
    }
    int *value;
    int size;
public:
    void print() {
        for(int i=0;i<size;i++) {
            cout<<value[i]<<" ";
        }
        cout<<endl;
    }
    DataSerial(){};
    DataSerial(int n)
    {
        size = n;
        value = new int[n]();
    }
    void setArray(int v[]) {
        value = v;
    }
    void setSize(int n) {
        size = n;
    }
    int* getValue() {
        return value;
    }
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
        return n1.second == n2.second ? n1.first > n2.first : n1.second > n2.second;
        //second -> time; first -> pid
    }
};

priority_queue<pair<int,int>,vector<pair<int,int>>,CompareQueue> pq;
zmq::socket_t *sock_req_;

class Monitor {
public:
    Monitor() {
    };
    Monitor (zmq::context_t &ctx, int arraySize, int procId, int procNum)
                     {
        pthread_mutex_init(&ack_mutex, NULL);
        pthread_mutex_init(&queue_mutex, NULL);
        pthread_mutex_init(&wait_mutex, NULL);
        ctx_ = &ctx;
        sock_req_ = new zmq::socket_t(*ctx_, ZMQ_DEALER);
        dataArray = new DataSerial(arraySize);
        PROC_NUM = procNum; //TODO to config
        prodMod = 0;
        consMod = 0;
        ltime = 0;
        pid = procId;
        size = arraySize;
        printMessage("VALUES INITIALIZED");
        sock_req_->connect(LOCAL_REQUEST_URL);
        printMessage("SOCKETS INITIALIZED");

    }
    void start_listening() {
        pthread_t handle_message_thread;
    }


    GenericMessage<pair<int,int>> prepare_message(int MSG_TYPE) {
        pair<int,int> p = make_pair(pid,ltime);
        GenericMessage<pair<int,int>> message = GenericMessage<pair<int,int>>(pid, MSG_TYPE, p);
        return message;
    }
    zmq::message_t prepare_empty(MSG_T TYPE,int mutexId){
        struct Message m;
        m.pid=pid;
        m.consMod = consMod;
        m.prodMod = prodMod;
        m.type = TYPE;
        m.mutexId = mutexId;
        m.time = ltime++; //++?
        zmq::message_t msg (sizeof(struct Message));
        memcpy (msg.data (), &m, (sizeof(struct Message)));
        return msg;
    }
    zmq::message_t prepare_ack(int mutexId, int destination) {
        struct Message m;
        m.pid=pid;
        m.consMod = consMod;
        m.prodMod = prodMod;
        m.type = ACK;
        m.destpid = destination;
        m.mutexId = mutexId;
        m.time = ltime++;
        zmq::message_t msg (sizeof(struct Message));
        memcpy (msg.data (), &m, (sizeof(struct Message)));
        return msg;
    }
    zmq::message_t prepare_message(MSG_T TYPE, int mutexId, int condId) {
        struct Message m;
        m.pid = pid;
        m.consMod = consMod;
        m.prodMod = prodMod;
        m.type = TYPE;
        m.mutexId = mutexId;
        m.condId = condId;
        m.time = ltime++; //++?
        ostringstream oss;
        boost::archive::text_oarchive oa(oss);
        oa << *dataArray;
        string serialized = oss.str();
        strcpy(m.dataString, serialized.c_str());
        zmq::message_t msg (sizeof(struct Message));
        memcpy (msg.data (), &m, (sizeof(struct Message)));

        return msg;
    }
    void print_serialized_data() {
        std::ostringstream oss;
        boost::archive::text_oarchive oa(oss);
        oa << *dataArray;
        string serialized = oss.str();
        printMessage(serialized);
    }
    void print_ack() {
        for(int i =0;i<PROC_NUM;i++) {
            cout<<ack[i]<<" ";
        }
        cout<<endl;
    }
    void lock(int mutexId) {
        sock_req_->send(prepare_empty(REQUEST,mutexId));
        printMessage("REQUEST SENT");
        cout<<ltime<<" "<<PROC_NUM<<endl;
        pthread_mutex_lock(&ack_mutex);
        ack = new int[PROC_NUM];
        memset(ack, 0, PROC_NUM*sizeof(*ack)); //zero ack table
        while(!isWaiting()) {
            pthread_cond_wait(&ack_cond, &ack_mutex);
        }
        pthread_mutex_unlock(&ack_mutex);
        pthread_mutex_lock(&queue_mutex);
        wait_for_queue(&queue_cond, &queue_mutex);
        pthread_mutex_unlock(&queue_mutex);
        printMessage("MUTEX LOCKED");
    }
    void wait_for_queue(pthread_cond_t *cond, pthread_mutex_t *mutex) {
        while(pq.top().first != pid) {
            pthread_cond_wait(&queue_cond, &queue_mutex);
        }
    }
    void wait(int condId, int mutexId) {
        unlock(mutexId);
        pthread_mutex_lock(&wait_mutex);

        pthread_cond_wait(&wait_cond,&wait_mutex);
        pthread_mutex_unlock(&wait_mutex);
        ltime--;
        lock(mutexId);
    }
    void signal(int condId) {
        sock_req_->send(prepare_message(SIGNAL,-1,condId));
        printMessage("SIGNALED");
    }
    void unlock(int mutexId) {
        pq.pop();
        sock_req_->send(prepare_message(RELEASE,mutexId, -1));
        printMessage("RELEASED");
    }
    static void* handle_message(void* m) {
        Monitor* _this = ((Monitor*)m);
        zmq::socket_t sock_sub_ (*((Monitor*)m)->ctx_, ZMQ_SUB);
        sock_sub_.connect(LOCAL_PUB_URL);
        sock_sub_.setsockopt(ZMQ_SUBSCRIBE, "", 0);
        while (true){
            zmq::message_t msg;

            sock_sub_.recv(&msg);
            struct Message *msg2 = (struct Message*) msg.data();

            switch(msg2->type){
                case ACK: {
                    if(msg2->destpid == _this->pid){
                        cout<<"+"<<msg2->pid<<endl;
                        pthread_mutex_lock(&(_this->ack_mutex));
                        (_this->ack)[msg2->pid] = 1;
                        pthread_cond_signal(&(_this->ack_cond));
                        pthread_mutex_unlock(&(_this->ack_mutex));
                    }
                    break;
                }
                case REQUEST:{
                    pair<int,int> temp = make_pair(msg2->pid,msg2->time);
                    pq.push(temp);
                    sock_req_->send(_this->prepare_ack(msg2->mutexId,msg2->pid));
                    break;
                }
                case RELEASE: {
                    if(_this->pid != msg2->pid){
                        pthread_mutex_lock(&(_this->queue_mutex));
                        pq.pop();
                        string sx(msg2->dataString);
                        std::istringstream iss(sx);
                        boost::archive::text_iarchive oa(iss);
                        DataSerial ds = DataSerial(100);
                        oa >> ds;
                        _this->dataArray = &ds;
                        _this->ltime = max(msg2->time+1, _this->ltime);
                        _this->prodMod = msg2->prodMod;
                        _this->consMod = msg2->consMod;
                        _this->printArray();
                        pthread_mutex_unlock(&(_this->queue_mutex));
                        pthread_cond_signal(&(_this->queue_cond));
                    }
                    break;
                }
                case SIGNAL: {
                    if(_this->pid != msg2->pid){
                        pthread_cond_signal(&(_this->wait_cond)); //TODO check if waiting/ diff between consumer/producer (between conds)
                    }
                    break;
                }
                default:
                    stringstream ss;
                    ss << "UNIDENTIFIED MSG TYPE -> " <<msg2->type<< " <-";
                    _this->printMessage(ss.str());
                    break;
            }
        }
    }
    string simpleMessage(struct Message* msg) {
        stringstream ss;
        ss << "RECIVED: Type " <<MSG_TStrings[msg->type] <<" | PID "<<msg->pid<<" | time "<<msg->time;
        return ss.str();
    }
    void printMessage(string m) {
        timeval curTime;
        gettimeofday(&curTime, NULL);
        int milli = curTime.tv_usec / 1000;

        char buffer [80];
        strftime(buffer, 80, "%Y-%m-%d %H:%M:%S", localtime(&curTime.tv_sec));

        char currentTime[84] = "";
        sprintf(currentTime, "%s:%d", buffer, milli);
        printf("%s ||| %s\n", currentTime,m.c_str());
    }
    void printArray() {
        dataArray->print();
    }
    void printQueue() {
        cout<<"QUEUE"<<endl;
        if(!pq.empty()) {
            pair<int,int> r = pq.top();
            cout<<"PID "<<r.first<<" | time "<<r.second<<endl;
        } else {
            cout<<"QUEUE EMPTY"<<endl;
        }
    }
protected:
    int *value;
    DataSerial *dataArray;
    int *ack;
    int pid;
    int ltime;
    int PROC_NUM;
    int size;
    int prodMod;
    int consMod;

    pthread_mutex_t ack_mutex;
    pthread_cond_t ack_cond = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t wait_mutex;
    pthread_cond_t wait_cond = PTHREAD_COND_INITIALIZER;
    pthread_mutex_t queue_mutex;
    pthread_cond_t queue_cond = PTHREAD_COND_INITIALIZER;
    zmq::context_t *ctx_;

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

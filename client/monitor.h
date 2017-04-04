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
    int time;
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
        return n1.second == n2.second ? n1.first < n2.first : n1.second < n2.second;
        //second -> time; first -> pid
    }
};

//template <class T>
class Monitor {
public:
    Monitor() {
    };
    Monitor (zmq::context_t &ctx, int arraySize, int procId)
                     {
            ctx_ = &ctx;
            sock_req_ = new zmq::socket_t(*ctx_, ZMQ_DEALER);
//        value = new int[arraySize]();
        dataArray = new DataSerial(arraySize);
        prodMod = 0;
        consMod = 0;
        ltime = 0;
        pid = procId;
        size = arraySize;
//        sock_req_.setsockopt(ZMQ_IDENTITY, pid, sizeof(pid));
        printMessage("VALUES INITIALIZED");
        sock_req_->connect(LOCAL_REQUEST_URL);
        printMessage("SOCKETS INITIALIZED");
//        sock_sub_.setsockopt(ZMQ_IDENTITY, "Hello", 5);
//        sock_sub_.setsockopt(ZMQ_SUBSCRIBE, "", 0);
//        sock_sub_.connect(LOCAL_PUB_URL);
//        printMessage("SOCKETS INITIALIZED");
//        printMessage("SYNCING");
//        zmq::socket_t sync(ctx_, ZMQ_PUSH);
//        sync.connect("tcp://localhost:5571");
//        s_send (sync, "");
//        sock_sub_.setsockopt(ZMQ_IDENTITY, pid, sizeof(pid));
//        const char *filter = (argc > 1)? argv [1]: "ACK_SUB";
//        sock_sub_.setsockopt(ZMQ_SUBSCRIBER, filter, strlen(filter));

    }
    void start_listening() {
        pthread_t handle_message_thread;
//        pthread_create(&handle_message_thread, NULL, &Monitor::handle_message, NULL);
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
    template<class Archive>
    void serialize(Archive & ar, unsigned int version)
    {
        int i;
        for(i = 0; i < size; ++i)
            ar & value[i];
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
    zmq::message_t prepare_empty(MSG_T TYPE,int mutexId){
        struct Message m;
        m.pid = pid;
        m.type = TYPE;
        m.mutexId = mutexId;
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
    zmq::message_t prepare_message(MSG_T TYPE, int mutexId, int condId) {
        struct Message m;
        m.pid = pid;
        m.type = TYPE;
        m.mutexId = mutexId;
        m.condId = condId;
        m.time = ltime++; //++?
        memcpy(m.data, value, size*sizeof(int));
        ostringstream oss;
        boost::archive::text_oarchive oa(oss);
        oa << *dataArray;
        string serialized = oss.str();
        cout<<"before "<<serialized<<endl;
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
    void lock(int mutexId) {
        //request for mutex
        sock_req_->send(prepare_empty(REQUEST,mutexId));
        printMessage("REQUEST SENT");
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
        sock_req_->send(prepare_message(SIGNAL,-1,condId));
        //with condId, mutexId, values
        //port 5570 request with value, consMod, prodMod, dont need to wait
    }
    void unlock(int mutexId) {
        pq.pop();
        sock_req_->send(prepare_message(RELEASE,mutexId, -1));
        //with mutexId, values
        //port 5570 request with value, consMod, prodMod, dont need to wait
    }
    static void* handle_message(void* m) {
        zmq::socket_t sock_sub_ (*((Monitor*)m)->ctx_, ZMQ_SUB);
        sock_sub_.connect(LOCAL_PUB_URL);
//        subscriber.setsockopt(ZMQ_SUBSCRIBE, "BROAD", 5);
//        sock_sub_.setsockopt(ZMQ_IDENTITY, "Hello", 5);
        sock_sub_.setsockopt(ZMQ_SUBSCRIBE, "", 0);

        //m->printMessage("HANDLER STARTED");
//        zmq::context_t context(1);
//        zmq::socket_t subscriber (context, ZMQ_SUB);
//        int i =0;
//        while (i<1000) {
//            std::string address = s_recv(subscriber);
//            std::string contents = s_recv(subscriber);
//            std::cout << "[" << address << "] " << contents << std::endl;
//        }
//        subscriber.connect("tcp://localhost:5563");
//        subscriber.setsockopt( ZMQ_SUBSCRIBE, "B", 1);
        while (true){
            zmq::message_t msg;
            Monitor* m = ((Monitor*)m);
            m->printMessage("WAITING");
            sock_sub_.recv(&msg);
//            m->printMessage(msg);
            struct Message *msg2 = (struct Message*) msg.data();
            m->printMessage(m->simpleMessage(msg2));
            switch(msg2->type){
                case ACK:
                    //fil array
                    //signal local cond ->waiting for ack

                    break;
                case REQUEST:
                    //add to PQ
                    //send ack
                    break;
                case RELEASE:
                    //update DATA
                    //signal local cond ->waiting for release
                    break;
                case SIGNAL:
                    //signal local where is waiting
                    break;
                default:
                    stringstream ss;
                    ss << "UNIDENTIFIED MSG TYPE -> " <<msg2->type<< " <-";
                    m->printMessage(ss.str());
                    break;
            }
        }
        //pthread process must loop this on sock_sub_.recv
        //and switch on type
    }
    string simpleMessage(struct Message* msg) {
        stringstream ss;
        ss << "RECIVED: Type " <<MSG_TStrings[msg->type] <<" | PID "<<msg->pid<<" | time "<<msg->time;
        return ss.str();
    }
    void printMessage(string m) {
        time_t rawtime;
        struct tm * timeinfo;
        time( &rawtime);
        timeinfo = localtime( &rawtime);
        std::cout<< asctime(timeinfo) <<m<<endl;
//        printf("%s\t | %s",asctime (timeinfo),m);
    }
    void printArray() {
        dataArray->print();
    }
private:
    int *value;
    DataSerial *dataArray;
    int *ack;
    int pid;
    int ltime;
    int PROC_NUM;
//    T value;
    int size;
    int prodMod;
    int consMod;
    zmq::context_t *ctx_;
    zmq::socket_t *sock_req_;
//    zmq::socket_t sock_sub_;
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

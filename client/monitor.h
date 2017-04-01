//
// Created by root on 4/1/17.
//
#include <sstream>
#ifndef DISTRIBUTEDMONITOR_MONITOR_H
#define DISTRIBUTEDMONITOR_MONITOR_H

#endif //DISTRIBUTEDMONITOR_MONITOR_H
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
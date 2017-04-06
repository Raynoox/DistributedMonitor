//
// Created by root on 4/6/17.
//

#ifndef DISTRIBUTEDMONITOR_CONFIGURATION_H
#define DISTRIBUTEDMONITOR_CONFIGURATION_H
#include<fstream>
#include<string>
#include <sstream>
#include <iostream>
class Config {

public:
    int PROC_NUM;
    list<string> addresses;
    string thisaddr;
    Config() {
        std::ifstream config("config.txt");
        if(config) {
            std::stringstream is_file;
            is_file << config.rdbuf();
            std::string line;
            while (std::getline(is_file, line)) {
                std::istringstream is_line(line);
                std::string key;
                if (std::getline(is_line, key, '=')) {
                    std::string value;
                    if (std::getline(is_line, value)){
                        store_line(key, value);
                    }
                }
            }
        }
        cout<<"PROCESSES NUMBER = "<<PROC_NUM<<endl;
        cout<<"PROXY NUMBER = "<<addresses.size()<<endl;
        for(std::list<string>::iterator list_iter = addresses.begin();
            list_iter != addresses.end(); list_iter++)
        {
            std::cout<<*list_iter<<endl;
        }
    }
    void store_line(string key, string value) {
        if(key == "proxy") {
            addresses.push_back(value);
        }
        if(key == "this") {
            thisaddr = value;
        }
        if(key == "proc_num") {
            PROC_NUM=atoi(value.c_str());
        }
    }

};

#endif //DISTRIBUTEDMONITOR_CONFIGURATION_H

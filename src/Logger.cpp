//
// Created by yknomeh on 11/16/18.
//
#include <iostream>
#include <fstream>
#include <ctime>
#include "Logger.h"

using namespace std;

const string LOG_FILE_PATH = "/home/yknomeh/.termlogs.love";

string get_message(string message) {
    auto time_now = time(nullptr);
    return ctime(&time_now) + message;
}

bool Logger::write(string message) {
    if (message == "") {
        return false;
    }

    ifstream f(LOG_FILE_PATH.c_str());

    if (!f.good()) {
        ofstream log_file(LOG_FILE_PATH);
        log_file << message << endl;

        log_file.close();
    } else {
        std::ofstream log_file;

        log_file.open(LOG_FILE_PATH, std::ios_base::app);
        log_file << get_message(message) << endl;
    }

    return true;
}
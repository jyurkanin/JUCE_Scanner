#include "log_value.h"


std::ofstream LOGFILE::log_file;

void LOGFILE::open_log(){
    log_file.open("/home/justin/juce_log.csv");
    log_file << "value\n";
}

void LOGFILE::log_value(float value){
    log_file << value << '\n';
}

void LOGFILE::close_log(){
    log_file.close();
}

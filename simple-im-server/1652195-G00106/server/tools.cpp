#include "tools.hpp"

string get_current_time()
{
    string ret;
    time_t now = time(0);
    tm *ltm = localtime(&now);
    char current_time[30];
    sprintf(current_time, "%d_%02d_%02d_%02d_%02d_%02d",ltm->tm_year + 1900, 
        ltm->tm_mon + 1, ltm->tm_mday, ltm->tm_hour, ltm->tm_min, ltm->tm_sec);
    return string(current_time);
}
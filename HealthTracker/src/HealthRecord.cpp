#include "HealthRecord.h"
#include <ctime>
#include <iomanip>
#include <sstream>

string HealthRecord::now_date() {
    time_t t =  time(nullptr);
    tm tm{};
    gmtime_r(&t, &tm);
    char buf[32];
    strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
    return  string(buf);
}

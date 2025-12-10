#ifndef HEALTH_RECORD_H
#define HEALTH_RECORD_H

#include <string>
#include <iostream>
using namespace std;

/**
 * @class HealthRecord
 * @brief Guarda la informacion de la forma indicada por los requisitos
 *
 * Guarda la fecha en formato "YYYY-MM-DD".
 */
class HealthRecord {
public:
    int id{ -1 };
    int user_id{ -1 };
    string datetime;
    double weight{ 0.0 };
    int systolic{ 0 };
    int diastolic{ 0 };
    double glucose{ 0.0 };

    HealthRecord() = default;


    static string now_date();
};

#endif

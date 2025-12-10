#ifndef CSV_EXPORTER_H
#define CSV_EXPORTER_H

#include <vector>
#include <string>
#include <iostream>
#include "HealthRecord.h"
using namespace std;


/**
 * @class CSVExporter
 * @brief exporta la tabla de base de datos hasta el momento
 */

class CSVExporter {
public:
    static bool export_file(const vector<HealthRecord>& records, const string& filename, const string& username, string& out_err, bool generate_graphs = true);
};

#endif

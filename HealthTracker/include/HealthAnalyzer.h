#ifndef HEALTH_ANALYZER_H
#define HEALTH_ANALYZER_H

#include <vector>
#include <optional>
#include "HealthRecord.h"
/**
 * @class HealthAnalizer
 * @brief Genera un pequeño analisis de la informacion adquirida
 */

class HealthAnalyzer {
public:
    static std::optional<double> average_weight(const std::vector<HealthRecord>& records);
    static std::pair<std::optional<double>, std::optional<double>> average_bp(const std::vector<HealthRecord>& records);
    static std::optional<double> average_glucose(const std::vector<HealthRecord>& records);
};

#endif


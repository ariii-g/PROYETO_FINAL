#include "HealthAnalyzer.h"

std::optional<double> HealthAnalyzer::average_weight(const std::vector<HealthRecord>& records) {
    if (records.empty()) return std::nullopt;
    double sum = 0.0;
    for (const auto& r : records) {
        sum += r.weight;
    }
    return sum / records.size();
}

std::pair<std::optional<double>, std::optional<double>> HealthAnalyzer::average_bp(const std::vector<HealthRecord>& records) {
    if (records.empty()) return { std::nullopt, std::nullopt };
    double s_sys = 0.0, s_dia = 0.0;
    for (const auto& r : records) {
        s_sys += r.systolic;
        s_dia += r.diastolic;
    }
    return { s_sys / records.size(), s_dia / records.size() };
}
std::optional<double> HealthAnalyzer::average_glucose(const std::vector<HealthRecord>& records) {
    if (records.empty()) return std::nullopt;
    double sum = 0.0;
    for (const auto& r : records) {
        sum += r.glucose;
    }
    return sum / records.size();
}

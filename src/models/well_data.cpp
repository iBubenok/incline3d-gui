#include "models/well_data.h"

#include <algorithm>
#include <unordered_map>

namespace incline3d::models {

std::string method_to_string(CalculationMethod method) {
    switch (method) {
        case CalculationMethod::kAverageAngle:
            return "average";
        case CalculationMethod::kBalancedTangential:
            return "balanced";
        case CalculationMethod::kMinimumCurvature:
            return "mincurv";
        case CalculationMethod::kRadiusOfCurvature:
            return "radiuscurv";
        case CalculationMethod::kRingArc:
            return "ringarc";
    }
    return "mincurv";
}

CalculationMethod string_to_method(const std::string& str) {
    static const std::unordered_map<std::string, CalculationMethod> mapping = {
        {"average", CalculationMethod::kAverageAngle},
        {"balanced", CalculationMethod::kBalancedTangential},
        {"mincurv", CalculationMethod::kMinimumCurvature},
        {"radiuscurv", CalculationMethod::kRadiusOfCurvature},
        {"ringarc", CalculationMethod::kRingArc},
        // Альтернативные названия
        {"average-angle", CalculationMethod::kAverageAngle},
        {"balanced-tangential", CalculationMethod::kBalancedTangential},
        {"minimum-curvature", CalculationMethod::kMinimumCurvature},
        {"radius-of-curvature", CalculationMethod::kRadiusOfCurvature},
        {"ring-arc", CalculationMethod::kRingArc},
    };

    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    auto it = mapping.find(lower);
    if (it != mapping.end()) {
        return it->second;
    }
    return CalculationMethod::kMinimumCurvature;
}

std::string azimuth_type_to_string(AzimuthType type) {
    switch (type) {
        case AzimuthType::kMagnetic:
            return "magnetic";
        case AzimuthType::kTrue:
            return "true";
        case AzimuthType::kGrid:
            return "grid";
    }
    return "magnetic";
}

AzimuthType string_to_azimuth_type(const std::string& str) {
    static const std::unordered_map<std::string, AzimuthType> mapping = {
        {"magnetic", AzimuthType::kMagnetic},
        {"true", AzimuthType::kTrue},
        {"grid", AzimuthType::kGrid},
        {"auto", AzimuthType::kMagnetic},  // авто = магнитный по умолчанию
    };

    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    auto it = mapping.find(lower);
    if (it != mapping.end()) {
        return it->second;
    }
    return AzimuthType::kMagnetic;
}

}  // namespace incline3d::models

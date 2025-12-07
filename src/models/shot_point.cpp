#include "models/shot_point.h"

#include <algorithm>
#include <unordered_map>

namespace incline3d::models {

std::string marker_to_string(ShotPointMarker marker) {
    switch (marker) {
        case ShotPointMarker::kTriangle:
            return "triangle";
        case ShotPointMarker::kSquare:
            return "square";
        case ShotPointMarker::kCircle:
            return "circle";
        case ShotPointMarker::kDiamond:
            return "diamond";
        case ShotPointMarker::kCross:
            return "cross";
    }
    return "triangle";
}

ShotPointMarker string_to_marker(const std::string& str) {
    static const std::unordered_map<std::string, ShotPointMarker> mapping = {
        {"triangle", ShotPointMarker::kTriangle},
        {"square", ShotPointMarker::kSquare},
        {"circle", ShotPointMarker::kCircle},
        {"diamond", ShotPointMarker::kDiamond},
        {"cross", ShotPointMarker::kCross},
    };

    std::string lower = str;
    std::transform(lower.begin(), lower.end(), lower.begin(), ::tolower);

    auto it = mapping.find(lower);
    if (it != mapping.end()) {
        return it->second;
    }
    return ShotPointMarker::kTriangle;
}

}  // namespace incline3d::models

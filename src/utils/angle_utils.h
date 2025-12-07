#pragma once

#include <QString>
#include <cmath>

namespace incline3d::utils {

/// Константы
constexpr double PI = 3.14159265358979323846;
constexpr double DEG_TO_RAD = PI / 180.0;
constexpr double RAD_TO_DEG = 180.0 / PI;

/// Конвертация градусов в радианы
inline double deg_to_rad(double deg) {
    return deg * DEG_TO_RAD;
}

/// Конвертация радианов в градусы
inline double rad_to_deg(double rad) {
    return rad * RAD_TO_DEG;
}

/// Нормализация угла в диапазон [0, 360)
inline double normalize_angle_360(double deg) {
    deg = std::fmod(deg, 360.0);
    if (deg < 0) {
        deg += 360.0;
    }
    return deg;
}

/// Нормализация угла в диапазон [-180, 180)
inline double normalize_angle_180(double deg) {
    deg = std::fmod(deg + 180.0, 360.0);
    if (deg < 0) {
        deg += 360.0;
    }
    return deg - 180.0;
}

/// Конвертация из формата "градусы.минуты" в десятичные градусы
/// Формат: XX.YY где YY — минуты (0-59) как сотые доли
/// Пример: 45.30 = 45°30' = 45.5° в десятичных
inline double deg_from_degmin(double degmin_value) {
    double deg = std::floor(degmin_value);
    double min = (degmin_value - deg) * 100.0;
    return deg + min / 60.0;
}

/// Алиас для совместимости с C++-ядром
inline double deg_from_degmin_value(double degmin_value) {
    return deg_from_degmin(degmin_value);
}

/// Конвертация из десятичных градусов в формат "градусы.минуты"
/// Пример: 45.5° = 45°30' = 45.30 в формате XX.YY
inline double deg_to_degmin(double decimal_deg) {
    double deg = std::floor(decimal_deg);
    double min = (decimal_deg - deg) * 60.0;
    return deg + min / 100.0;
}

/// Алиас для совместимости с C++-ядром
inline double deg_to_degmin_value(double decimal_deg) {
    return deg_to_degmin(decimal_deg);
}

/// Форматирование угла как строки (десятичные градусы)
inline QString format_angle_decimal(double deg, int precision = 2) {
    return QString::number(deg, 'f', precision) + QString::fromUtf8("°");
}

/// Форматирование угла как строки (градусы и минуты)
inline QString format_angle_degmin(double decimal_deg, int precision = 1) {
    double deg = std::floor(decimal_deg);
    double min = (decimal_deg - deg) * 60.0;
    return QString::number(static_cast<int>(deg)) + QString::fromUtf8("°") +
           QString::number(min, 'f', precision) + "'";
}

/// Форматирование угла как строки (градусы, минуты, секунды)
inline QString format_angle_dms(double decimal_deg) {
    double deg = std::floor(decimal_deg);
    double min_total = (decimal_deg - deg) * 60.0;
    double min = std::floor(min_total);
    double sec = (min_total - min) * 60.0;
    return QString::number(static_cast<int>(deg)) + QString::fromUtf8("°") +
           QString::number(static_cast<int>(min)) + "'" +
           QString::number(sec, 'f', 1) + "\"";
}

/// Парсинг угла из строки (поддерживает разные форматы)
/// Возвращает true если парсинг успешен
bool parse_angle(const QString& str, double& result);

/// Расчёт азимута между двумя точками (север = 0, по часовой)
inline double calculate_azimuth(double dx, double dy) {
    double azim = std::atan2(dx, dy) * RAD_TO_DEG;
    return normalize_angle_360(azim);
}

/// Расчёт расстояния между двумя точками
inline double calculate_distance(double dx, double dy) {
    return std::sqrt(dx * dx + dy * dy);
}

/// Расчёт 3D расстояния
inline double calculate_distance_3d(double dx, double dy, double dz) {
    return std::sqrt(dx * dx + dy * dy + dz * dz);
}

}  // namespace incline3d::utils

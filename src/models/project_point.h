#pragma once

#include <string>
#include <vector>
#include <QColor>

namespace incline3d::models {

/// Проектная точка (точка пласта для контроля попадания)
struct ProjectPoint {
    std::string name;                       ///< Название пласта/точки
    double azimuth_geogr_deg{0.0};          ///< Географический азимут, градусы
    double shift_m{0.0};                    ///< Горизонтальное смещение, м
    double depth_m{0.0};                    ///< Глубина по стволу, м
    double abs_depth_m{0.0};                ///< Абсолютная глубина, м
    double radius_m{0.0};                   ///< Радиус допуска, м

    // Базовые параметры (опорная точка)
    double base_azim_geogr_deg{0.0};        ///< Базовый географический азимут
    double base_shift_m{0.0};               ///< Базовое смещение
    double base_depth_m{0.0};               ///< Базовая глубина

    // Фактические значения (заполняются после расчёта)
    double fact_inclination_deg{0.0};       ///< Фактический угол наклона
    double fact_azimuth_deg{0.0};           ///< Фактический магнитный азимут
    double fact_true_azimuth_deg{0.0};      ///< Фактический истинный азимут
    double fact_shift_m{0.0};               ///< Фактическое смещение
    double fact_length_m{0.0};              ///< Фактическая длина
    double fact_north_m{0.0};               ///< Фактическое смещение на север
    double fact_east_m{0.0};                ///< Фактическое смещение на восток
    double fact_offset_m{0.0};              ///< Фактическое отклонение от проекта
    double fact_geo_azimuth_offset_deg{0.0};///< Азимут отклонения
    double fact_tvd_m{0.0};                 ///< Фактическая вертикальная глубина
    double fact_intensity_10m{0.0};         ///< Фактическая интенсивность на 10 м
    double fact_intensity_L{0.0};           ///< Фактическая интенсивность на L

    // Визуальные настройки
    QColor display_color{Qt::red};          ///< Цвет отображения
    bool visible{true};                     ///< Видимость
    bool show_tolerance_circle{true};       ///< Показывать круг допуска
};

/// Массив проектных точек с общими настройками
struct ProjectPointsSet {
    std::vector<ProjectPoint> points;
    std::string associated_well;            ///< Связанная скважина (если есть)

    // Общие настройки отображения
    bool show_labels{true};                 ///< Показывать подписи
    bool show_depth_labels{true};           ///< Показывать глубины
    int label_font_size{10};                ///< Размер шрифта подписей
};

}  // namespace incline3d::models

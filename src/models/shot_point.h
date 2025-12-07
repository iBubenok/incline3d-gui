#pragma once

#include <string>
#include <vector>
#include <QColor>

namespace incline3d::models {

/// Тип маркера пункта возбуждения
enum class ShotPointMarker {
    kTriangle,          ///< Треугольник
    kSquare,            ///< Квадрат
    kCircle,            ///< Круг
    kDiamond,           ///< Ромб
    kCross              ///< Крест
};

/// Пункт возбуждения (сейсмика)
struct ShotPoint {
    std::string name;                       ///< Название/номер пункта
    double x_m{0.0};                        ///< Координата X (восток), м
    double y_m{0.0};                        ///< Координата Y (север), м
    double z_m{0.0};                        ///< Координата Z (глубина/высота), м

    // Визуальные настройки
    QColor display_color{Qt::green};        ///< Цвет отображения
    ShotPointMarker marker{ShotPointMarker::kTriangle}; ///< Тип маркера
    int marker_size{10};                    ///< Размер маркера
    bool visible{true};                     ///< Видимость
    bool show_label{true};                  ///< Показывать подпись
};

/// Набор пунктов возбуждения с общими настройками
struct ShotPointsSet {
    std::vector<ShotPoint> points;

    // Общие настройки отображения
    bool show_labels{true};                 ///< Показывать подписи
    int default_marker_size{10};            ///< Размер маркера по умолчанию
    int label_font_size{9};                 ///< Размер шрифта подписей
};

/// Конвертация типа маркера в строку
std::string marker_to_string(ShotPointMarker marker);

/// Конвертация строки в тип маркера
ShotPointMarker string_to_marker(const std::string& str);

}  // namespace incline3d::models

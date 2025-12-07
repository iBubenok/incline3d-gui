#pragma once

#include <QColor>

namespace incline3d::views {

/// Настройки визуализации
struct ViewSettings {
    // Цвета
    QColor background_color{Qt::white};
    QColor grid_color{200, 200, 200};
    QColor axis_x_color{Qt::red};
    QColor axis_y_color{Qt::green};
    QColor axis_z_color{Qt::blue};
    QColor sea_level_color{100, 150, 255, 100};

    // Сетка
    bool show_grid{true};
    double grid_step{100.0};
    int grid_divisions{10};

    // Оси
    bool show_axes{true};
    double axis_length{500.0};

    // Глубины
    bool show_depth_labels{true};
    double depth_label_step{100.0};
    int depth_label_font_size{10};

    // Проектные точки
    bool show_project_points{true};
    bool show_tolerance_circles{true};

    // Пункты возбуждения
    bool show_shot_points{true};

    // Уровень моря
    bool show_sea_level{false};
    double sea_level_elevation{0.0};
};

}  // namespace incline3d::views

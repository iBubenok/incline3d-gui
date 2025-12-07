#pragma once

#include <QDialog>
#include <QColor>

class QSpinBox;
class QDoubleSpinBox;
class QCheckBox;
class QComboBox;
class QPushButton;
class QTabWidget;

namespace incline3d::ui {

/// Настройки отображения видов
struct ViewOptions {
    // Общие настройки
    bool show_grid{true};
    bool show_axes{true};
    bool show_labels{true};
    bool show_depth_marks{true};
    double grid_step{100.0};              ///< Шаг сетки, м
    double depth_label_step{500.0};       ///< Шаг отметок глубины, м

    // Настройки траекторий
    double default_line_width{2.0};
    bool show_wellhead{true};
    bool show_trajectory_points{false};

    // Настройки проектных точек
    bool show_project_points{true};
    bool show_tolerance_circles{true};
    double tolerance_circle_alpha{0.3};

    // Настройки пунктов возбуждения
    bool show_shot_points{true};
    double shot_point_size{10.0};

    // 3D специфичные
    bool show_sea_level_plane{true};
    QColor sea_level_color{0, 100, 200, 80};
    bool enable_perspective{true};
    double rotation_sensitivity{1.0};

    // Вертикальная проекция
    bool auto_fit_azimuth{true};
    double profile_azimuth{0.0};

    // Цвета
    QColor background_color{Qt::white};
    QColor grid_color{200, 200, 200};
    QColor axis_x_color{Qt::red};
    QColor axis_y_color{Qt::green};
    QColor axis_z_color{Qt::blue};
};

/// Диалог настроек отображения видов
class ViewOptionsDialog : public QDialog {
    Q_OBJECT

public:
    explicit ViewOptionsDialog(QWidget* parent = nullptr);
    ~ViewOptionsDialog() override = default;

    /// Установить настройки
    void setOptions(const ViewOptions& options);

    /// Получить настройки
    ViewOptions options() const;

signals:
    /// Сигнал о применении настроек (для предпросмотра)
    void optionsChanged(const ViewOptions& options);

private slots:
    void onApply();
    void onColorButton();

private:
    void setupUi();
    void setupGeneralTab(QWidget* tab);
    void setupTrajectoryTab(QWidget* tab);
    void setupPointsTab(QWidget* tab);
    void setup3DTab(QWidget* tab);
    void setupColorsTab(QWidget* tab);

    void updateColorButton(QPushButton* button, const QColor& color);

    QTabWidget* tab_widget_{nullptr};

    // Общие
    QCheckBox* show_grid_check_{nullptr};
    QCheckBox* show_axes_check_{nullptr};
    QCheckBox* show_labels_check_{nullptr};
    QCheckBox* show_depth_marks_check_{nullptr};
    QDoubleSpinBox* grid_step_spin_{nullptr};
    QDoubleSpinBox* depth_label_step_spin_{nullptr};

    // Траектории
    QDoubleSpinBox* line_width_spin_{nullptr};
    QCheckBox* show_wellhead_check_{nullptr};
    QCheckBox* show_points_check_{nullptr};

    // Точки
    QCheckBox* show_project_points_check_{nullptr};
    QCheckBox* show_tolerance_check_{nullptr};
    QDoubleSpinBox* tolerance_alpha_spin_{nullptr};
    QCheckBox* show_shot_points_check_{nullptr};
    QDoubleSpinBox* shot_size_spin_{nullptr};

    // 3D
    QCheckBox* show_sea_level_check_{nullptr};
    QCheckBox* perspective_check_{nullptr};
    QDoubleSpinBox* rotation_sens_spin_{nullptr};

    // Вертикальная проекция
    QCheckBox* auto_azimuth_check_{nullptr};
    QDoubleSpinBox* azimuth_spin_{nullptr};

    // Цвета
    QPushButton* bg_color_button_{nullptr};
    QPushButton* grid_color_button_{nullptr};
    QPushButton* axis_x_button_{nullptr};
    QPushButton* axis_y_button_{nullptr};
    QPushButton* axis_z_button_{nullptr};
    QPushButton* sea_level_button_{nullptr};

    // Текущие цвета
    QColor bg_color_;
    QColor grid_color_;
    QColor axis_x_color_;
    QColor axis_y_color_;
    QColor axis_z_color_;
    QColor sea_level_color_;
};

}  // namespace incline3d::ui

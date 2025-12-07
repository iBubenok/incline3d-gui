#pragma once

#include <QDialog>
#include <memory>

#include "models/well_data.h"

class QComboBox;
class QDoubleSpinBox;
class QSpinBox;
class QCheckBox;
class QLineEdit;
class QTextEdit;
class QTabWidget;
class QProgressBar;
class QPushButton;
class QGroupBox;

namespace incline3d::core {
class InclineProcessRunner;
}

namespace incline3d::ui {

/// Диалог обработки скважины
/// Реализует полную функциональность "Обработка → Обработка" из PrimeINCL
class ProcessDialog : public QDialog {
    Q_OBJECT

public:
    ProcessDialog(std::shared_ptr<models::WellData> well,
                  core::InclineProcessRunner* runner,
                  QWidget* parent = nullptr);

private slots:
    void onProcess();
    void onProcessFinished();
    void onAzimuthModeChanged(int index);
    void onSngfModeChanged(bool enabled);

private:
    void setupUi();
    void createMethodTab();
    void createAzimuthTab();
    void createElevationTab();
    void createQualityTab();
    void loadParams();
    void saveParams();

    std::shared_ptr<models::WellData> well_;
    core::InclineProcessRunner* runner_;

    QTabWidget* tab_widget_{nullptr};

    // === Вкладка "Метод" ===
    QComboBox* method_combo_{nullptr};
    QDoubleSpinBox* intensity_interval_spin_{nullptr};
    QCheckBox* smooth_intensity_check_{nullptr};

    // === Вкладка "Азимуты" ===
    QComboBox* azimuth_type_combo_{nullptr};
    QDoubleSpinBox* declination_spin_{nullptr};
    QDoubleSpinBox* meridian_spin_{nullptr};
    QCheckBox* use_summary_correction_check_{nullptr};

    // Режим обработки пропущенных азимутов
    QComboBox* blank_azimuth_mode_combo_{nullptr};
    QCheckBox* use_last_azimuth_check_{nullptr};
    QCheckBox* interpolate_azimuths_check_{nullptr};
    QCheckBox* unwrap_azimuths_check_{nullptr};
    QCheckBox* continuous_mode_check_{nullptr};

    // SNGF-режим
    QCheckBox* sngf_mode_check_{nullptr};
    QDoubleSpinBox* sngf_min_angle_spin_{nullptr};

    // Вертикальный режим
    QDoubleSpinBox* vertical_limit_spin_{nullptr};

    // === Вкладка "Высотные отметки" ===
    QDoubleSpinBox* kelly_bushing_spin_{nullptr};
    QDoubleSpinBox* ground_elevation_spin_{nullptr};
    QDoubleSpinBox* water_depth_spin_{nullptr};
    QCheckBox* calculate_tvd_bgl_check_{nullptr};
    QCheckBox* calculate_tvd_bml_check_{nullptr};

    // === Вкладка "Контроль качества" ===
    QCheckBox* quality_check_check_{nullptr};
    QDoubleSpinBox* max_angle_deviation_spin_{nullptr};
    QDoubleSpinBox* max_azimuth_deviation_spin_{nullptr};
    QDoubleSpinBox* intensity_threshold_spin_{nullptr};
    QDoubleSpinBox* delta_depth_warning_spin_{nullptr};

    // Погрешности измерений
    QDoubleSpinBox* error_depth_spin_{nullptr};
    QDoubleSpinBox* error_angle_spin_{nullptr};
    QDoubleSpinBox* error_azimuth_spin_{nullptr};

    // Лог и прогресс
    QTextEdit* log_text_{nullptr};
    QProgressBar* progress_bar_{nullptr};
    QPushButton* process_btn_{nullptr};
};

}  // namespace incline3d::ui

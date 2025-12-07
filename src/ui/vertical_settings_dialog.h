#pragma once

#include <QDialog>

class QDoubleSpinBox;
class QComboBox;
class QCheckBox;
class QLineEdit;
class QSpinBox;
class QPushButton;
class QLabel;

namespace incline3d::ui {

/// Настройки вертикальной проекции
struct VerticalProjectionSettings {
    double azimuth_deg{0.0};        ///< Азимут профиля, градусы
    bool auto_fit_azimuth{true};    ///< Автоподбор оптимального азимута
    double horizontal_scale{100.0}; ///< Горизонтальный масштаб (м на см)
    double vertical_scale{100.0};   ///< Вертикальный масштаб (м на см)
    bool link_scales{true};         ///< Связать масштабы
    double grid_step{50.0};         ///< Шаг сетки, м
    bool show_grid{true};           ///< Показывать сетку
    bool show_depth_labels{true};   ///< Показывать подписи глубин
    bool show_project_points{true}; ///< Показывать проектные точки
    bool show_sea_level{true};      ///< Показывать уровень моря
    double kelly_bushing{0.0};      ///< Альтитуда стола ротора (для уровня моря)
    int depth_label_step{100};      ///< Шаг подписей глубин, м

    // Шапка вертикальной проекции
    QString header_title;           ///< Заголовок
    QString header_well;            ///< Скважина
    QString header_field;           ///< Месторождение
    QString header_pad;             ///< Куст
    QString header_date;            ///< Дата
    QString header_scale;           ///< Масштаб (текст)
};

/// Диалог настройки параметров вертикальной проекции
/// Реализует функциональность настройки графиков из PrimeINCL
class VerticalSettingsDialog : public QDialog {
    Q_OBJECT

public:
    explicit VerticalSettingsDialog(QWidget* parent = nullptr);
    ~VerticalSettingsDialog() override;

    /// Получить настройки
    VerticalProjectionSettings settings() const;

    /// Установить настройки
    void setSettings(const VerticalProjectionSettings& settings);

signals:
    void settingsChanged(const VerticalProjectionSettings& settings);

private slots:
    void onAutoFitChanged(bool enabled);
    void onLinkScalesChanged(bool linked);
    void onHorizontalScaleChanged(double value);
    void onApply();

private:
    void setupUi();
    void createProjectionTab();
    void createHeaderTab();
    void emitSettings();

    // Вкладка "Проекция"
    QDoubleSpinBox* azimuth_spin_{nullptr};
    QCheckBox* auto_fit_check_{nullptr};
    QPushButton* fit_now_btn_{nullptr};

    QDoubleSpinBox* h_scale_spin_{nullptr};
    QDoubleSpinBox* v_scale_spin_{nullptr};
    QCheckBox* link_scales_check_{nullptr};

    QDoubleSpinBox* grid_step_spin_{nullptr};
    QCheckBox* show_grid_check_{nullptr};
    QCheckBox* show_depth_labels_check_{nullptr};
    QSpinBox* depth_label_step_spin_{nullptr};

    QCheckBox* show_project_points_check_{nullptr};
    QCheckBox* show_sea_level_check_{nullptr};
    QDoubleSpinBox* kelly_bushing_spin_{nullptr};

    // Вкладка "Шапка"
    QLineEdit* header_title_edit_{nullptr};
    QLineEdit* header_well_edit_{nullptr};
    QLineEdit* header_field_edit_{nullptr};
    QLineEdit* header_pad_edit_{nullptr};
    QLineEdit* header_date_edit_{nullptr};
    QLineEdit* header_scale_edit_{nullptr};
};

}  // namespace incline3d::ui

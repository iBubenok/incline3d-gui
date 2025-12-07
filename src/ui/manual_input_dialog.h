#pragma once

#include <QDialog>
#include <memory>

#include "models/well_data.h"

class QTabWidget;
class QTableView;
class QLineEdit;
class QDoubleSpinBox;
class QDateEdit;
class QComboBox;
class QCheckBox;
class QPushButton;
class QLabel;

namespace incline3d {

namespace models {
class MeasurementsModel;
}

namespace ui {

/// Режим отображения углов
enum class AngleDisplayMode {
    DecimalDegrees,     ///< Десятичные градусы (45.5°)
    DegreesMinutes      ///< Градусы и минуты (45.30 = 45°30')
};

/// Диалог ручного ввода исходных данных инклинометрии
/// Реализует функциональность "Исходные данные → Ручной ввод" из PrimeINCL
class ManualInputDialog : public QDialog {
    Q_OBJECT

public:
    explicit ManualInputDialog(QWidget* parent = nullptr);
    explicit ManualInputDialog(std::shared_ptr<models::WellData> well, QWidget* parent = nullptr);
    ~ManualInputDialog() override;

    /// Получить данные скважины после ввода
    std::shared_ptr<models::WellData> wellData() const;

    /// Установить данные для редактирования
    void setWellData(std::shared_ptr<models::WellData> well);

private slots:
    void onSave();
    void onCancel();
    void onAddRow();
    void onRemoveRow();
    void onImportFromClipboard();
    void onFlipArray();
    void onFlipColumn();
    void onReplacePseudoEmpty();
    void onToggleAngleUnits();
    void onValidateData();
    void onCellChanged(const QModelIndex& index);

private:
    void setupUi();
    void createFieldsTab();
    void createArrayTab();
    void loadFromWell();
    void saveToWell();
    bool validateInput();
    void updateErrorMessages();
    void highlightErrors();

    /// Конвертация угла из текущего режима отображения в десятичные градусы
    double angleToDecimal(double value) const;
    /// Конвертация угла из десятичных градусов в текущий режим отображения
    double angleFromDecimal(double value) const;

    std::shared_ptr<models::WellData> well_;
    std::unique_ptr<models::MeasurementsModel> measurements_model_;

    QTabWidget* tab_widget_{nullptr};

    // === Вкладка "Поля" ===
    QWidget* fields_tab_{nullptr};

    // Идентификация скважины
    QLineEdit* uwi_edit_{nullptr};
    QLineEdit* region_edit_{nullptr};
    QLineEdit* field_edit_{nullptr};
    QLineEdit* area_edit_{nullptr};
    QLineEdit* pad_edit_{nullptr};
    QLineEdit* well_edit_{nullptr};
    QLineEdit* measurement_number_edit_{nullptr};

    // Прибор
    QLineEdit* device_edit_{nullptr};
    QLineEdit* device_number_edit_{nullptr};
    QDateEdit* device_calibration_date_{nullptr};

    // Интервал и параметры
    QDoubleSpinBox* interval_start_spin_{nullptr};
    QDoubleSpinBox* interval_end_spin_{nullptr};
    QDoubleSpinBox* mag_declination_spin_{nullptr};
    QDoubleSpinBox* kelly_bushing_spin_{nullptr};
    QDoubleSpinBox* casing_shoe_spin_{nullptr};

    // Параметры скважины
    QDoubleSpinBox* d_casing_spin_{nullptr};
    QDoubleSpinBox* d_collar_spin_{nullptr};
    QDoubleSpinBox* current_depth_spin_{nullptr};
    QDoubleSpinBox* project_depth_spin_{nullptr};

    // Проектные параметры забоя
    QDoubleSpinBox* project_shift_spin_{nullptr};
    QDoubleSpinBox* project_azimuth_spin_{nullptr};
    QDoubleSpinBox* tolerance_radius_spin_{nullptr};

    // Погрешности измерений
    QDoubleSpinBox* angle_error_spin_{nullptr};
    QDoubleSpinBox* azimuth_error_spin_{nullptr};

    // Организационные данные
    QDateEdit* research_date_{nullptr};
    QLineEdit* conditions_edit_{nullptr};
    QLineEdit* research_type_edit_{nullptr};
    QComboBox* quality_combo_{nullptr};
    QLineEdit* customer_edit_{nullptr};
    QLineEdit* contractor_edit_{nullptr};
    QLineEdit* interpreter_edit_{nullptr};
    QLineEdit* party_chief_edit_{nullptr};

    // === Вкладка "Массив" ===
    QWidget* array_tab_{nullptr};
    QTableView* measurements_table_{nullptr};

    // Панель инструментов массива
    QPushButton* add_row_btn_{nullptr};
    QPushButton* remove_row_btn_{nullptr};
    QPushButton* import_clipboard_btn_{nullptr};
    QPushButton* flip_array_btn_{nullptr};
    QPushButton* flip_column_btn_{nullptr};
    QPushButton* replace_empty_btn_{nullptr};
    QComboBox* angle_units_combo_{nullptr};

    // Диагностика ошибок
    QLabel* error_label_{nullptr};
    QLabel* points_count_label_{nullptr};

    // Настройки
    AngleDisplayMode angle_mode_{AngleDisplayMode::DecimalDegrees};
    double max_angle_{120.0};  ///< Максимальный допустимый угол (Umax)
};

}  // namespace ui
}  // namespace incline3d

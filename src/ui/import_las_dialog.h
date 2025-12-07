#pragma once

#include <QDialog>
#include <memory>

#include "models/well_data.h"

class QLineEdit;
class QComboBox;
class QTableWidget;
class QCheckBox;
class QPushButton;
class QLabel;
class QTextEdit;

namespace incline3d {

namespace core {
class InclineProcessRunner;
}

namespace ui {

/// Диалог импорта данных из LAS-файла
/// Реализует функциональность "Исходные данные → Импорт из LAS" из PrimeINCL
class ImportLasDialog : public QDialog {
    Q_OBJECT

public:
    explicit ImportLasDialog(const QString& file_path, QWidget* parent = nullptr);
    ~ImportLasDialog() override;

    /// Получить импортированные данные скважины
    std::shared_ptr<models::WellData> wellData() const;

    /// Был ли импорт успешным
    bool isImportSuccessful() const { return import_successful_; }

private slots:
    void onLoadFile();
    void onCurveSelectionChanged();
    void onPreviewData();
    void onImport();

private:
    void setupUi();
    void parseLasFile();
    void updatePreview();
    void populateCurveComboBoxes();

    QString file_path_;
    std::shared_ptr<models::WellData> well_;
    bool import_successful_{false};

    // LAS-данные
    struct LasData {
        QString well_name;
        QString field;
        QString uwi;
        double null_value{-999.25};
        QStringList curve_names;
        QVector<QVector<double>> curve_data;
    };
    LasData las_data_;

    // Виджеты
    QLineEdit* file_path_edit_{nullptr};
    QPushButton* browse_btn_{nullptr};
    QLabel* status_label_{nullptr};

    // Информация о скважине из LAS
    QLineEdit* well_name_edit_{nullptr};
    QLineEdit* field_edit_{nullptr};
    QLineEdit* uwi_edit_{nullptr};

    // Выбор кривых
    QComboBox* depth_curve_combo_{nullptr};
    QComboBox* angle_curve_combo_{nullptr};
    QComboBox* azimuth_curve_combo_{nullptr};
    QCheckBox* angle_degmin_check_{nullptr};
    QCheckBox* azimuth_degmin_check_{nullptr};

    // Предпросмотр и лог
    QTableWidget* preview_table_{nullptr};
    QTextEdit* log_text_{nullptr};
    QLabel* points_count_label_{nullptr};
};

}  // namespace ui
}  // namespace incline3d

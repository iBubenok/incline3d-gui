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
class QSpinBox;

namespace incline3d::ui {

/// Диалог импорта данных из текстового файла ЗАК (Заключение по контролю)
/// Реализует функциональность "Исходные данные → Импорт из ЗАК" из PrimeINCL
class ImportZakDialog : public QDialog {
    Q_OBJECT

public:
    explicit ImportZakDialog(const QString& file_path, QWidget* parent = nullptr);
    ~ImportZakDialog() override;

    /// Получить импортированные данные скважины
    std::shared_ptr<models::WellData> wellData() const;

    /// Был ли импорт успешным
    bool isImportSuccessful() const { return import_successful_; }

private slots:
    void onLoadFile();
    void onSettingsChanged();
    void onImport();

private:
    void setupUi();
    void parseFile();
    void updatePreview();

    QString file_path_;
    std::shared_ptr<models::WellData> well_;
    bool import_successful_{false};

    // Исходные данные файла
    QStringList file_lines_;

    // Виджеты
    QLineEdit* file_path_edit_{nullptr};
    QPushButton* browse_btn_{nullptr};

    // Настройки парсера
    QComboBox* separator_combo_{nullptr};
    QSpinBox* skip_lines_spin_{nullptr};
    QSpinBox* depth_col_spin_{nullptr};
    QSpinBox* angle_col_spin_{nullptr};
    QSpinBox* azimuth_col_spin_{nullptr};
    QCheckBox* angle_degmin_check_{nullptr};
    QCheckBox* azimuth_degmin_check_{nullptr};
    QComboBox* decimal_separator_combo_{nullptr};

    // Метаданные
    QLineEdit* well_name_edit_{nullptr};

    // Предпросмотр
    QTableWidget* preview_table_{nullptr};
    QTextEdit* log_text_{nullptr};
    QLabel* points_count_label_{nullptr};
};

}  // namespace incline3d::ui

#pragma once

#include <QDialog>
#include <memory>
#include <vector>

class QLineEdit;
class QComboBox;
class QTextEdit;
class QCheckBox;
class QDoubleSpinBox;
class QPushButton;
class QTabWidget;
class QTableWidget;
class QDateEdit;
class QLabel;

namespace incline3d {

namespace models {
struct WellData;
struct ProjectPoint;
}  // namespace models

namespace ui {

/// Структура для хранения данных шапки заключения
struct ConclusionHeader {
    QString company;           ///< Организация (компания)
    QString field;             ///< Месторождение
    QString area;              ///< Площадь
    QString pad;               ///< Куст
    QString well;              ///< Скважина
    QString measurement_number;///< Номер измерения
    QString date;              ///< Дата
    QString device;            ///< Прибор
    QString device_number;     ///< Номер прибора
    QString device_calibration;///< Поверка прибора
    QString research_type;     ///< Вид исследования
    QString quality;           ///< Качество
    QString operator_name;     ///< Оператор
    QString interpreter;       ///< Интерпретатор
    QString party_chief;       ///< Начальник партии
    QString customer;          ///< Заказчик

    // Логотипы (пути к изображениям)
    QString logo_left;
    QString logo_right;
};

/// Диалог формирования заключения по результатам инклинометрии
/// Реализует функциональность "Заключение → Формирование" из PrimeINCL
class ConclusionDialog : public QDialog {
    Q_OBJECT

public:
    explicit ConclusionDialog(
        std::shared_ptr<models::WellData> well,
        const std::vector<models::ProjectPoint>& project_points,
        QWidget* parent = nullptr);
    ~ConclusionDialog() override;

    /// Получить данные шапки заключения
    ConclusionHeader header() const;

    /// Установить данные шапки
    void setHeader(const ConclusionHeader& header);

private slots:
    void onExportCsv();
    void onExportExcel();
    void onPrint();
    void onPreview();
    void onSelectLogoLeft();
    void onSelectLogoRight();
    void onClearLogoLeft();
    void onClearLogoRight();

private:
    void setupUi();
    void createHeaderTab();
    void createResultsTab();
    void createProjectPointsTab();
    void createSummaryTab();
    void loadFromWell();
    void updateSummary();
    QString generateCsvContent() const;
    QString generateHtmlReport() const;

    std::shared_ptr<models::WellData> well_;
    std::vector<models::ProjectPoint> project_points_;

    QTabWidget* tab_widget_{nullptr};

    // Вкладка "Шапка"
    QWidget* header_tab_{nullptr};
    QLineEdit* company_edit_{nullptr};
    QLineEdit* field_edit_{nullptr};
    QLineEdit* area_edit_{nullptr};
    QLineEdit* pad_edit_{nullptr};
    QLineEdit* well_edit_{nullptr};
    QLineEdit* measurement_number_edit_{nullptr};
    QDateEdit* date_edit_{nullptr};
    QLineEdit* device_edit_{nullptr};
    QLineEdit* device_number_edit_{nullptr};
    QLineEdit* calibration_edit_{nullptr};
    QLineEdit* research_type_edit_{nullptr};
    QComboBox* quality_combo_{nullptr};
    QLineEdit* operator_edit_{nullptr};
    QLineEdit* interpreter_edit_{nullptr};
    QLineEdit* party_chief_edit_{nullptr};
    QLineEdit* customer_edit_{nullptr};
    QLineEdit* logo_left_edit_{nullptr};
    QLineEdit* logo_right_edit_{nullptr};

    // Вкладка "Результаты"
    QWidget* results_tab_{nullptr};
    QTableWidget* results_table_{nullptr};

    // Вкладка "Проектные точки"
    QWidget* project_tab_{nullptr};
    QTableWidget* project_table_{nullptr};

    // Вкладка "Сводка"
    QWidget* summary_tab_{nullptr};
    QTextEdit* summary_text_{nullptr};

    // Настройки экспорта
    QCheckBox* include_header_check_{nullptr};
    QCheckBox* include_logo_check_{nullptr};
    QCheckBox* include_project_points_check_{nullptr};
    QComboBox* angle_format_combo_{nullptr};
};

}  // namespace ui
}  // namespace incline3d

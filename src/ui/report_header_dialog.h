#pragma once

#include <QDialog>
#include <QString>

class QLineEdit;
class QTextEdit;
class QDateEdit;
class QPushButton;
class QLabel;

namespace incline3d::ui {

/// Структура данных шапки отчёта
struct ReportHeader {
    QString organization;           ///< Организация-заказчик
    QString contractor;             ///< Организация-исполнитель
    QString field_name;             ///< Месторождение
    QString well_pad;               ///< Куст
    QString well_name;              ///< Скважина
    QString survey_type;            ///< Тип замера (инклинометрия)
    QString survey_date;            ///< Дата замера
    QString report_number;          ///< Номер заключения
    QString report_date;            ///< Дата заключения
    QString operator_name;          ///< ФИО оператора
    QString geologist_name;         ///< ФИО геолога
    QString notes;                  ///< Примечания
    QString logo_path;              ///< Путь к логотипу
};

/// Диалог редактирования шапки отчёта
class ReportHeaderDialog : public QDialog {
    Q_OBJECT

public:
    explicit ReportHeaderDialog(QWidget* parent = nullptr);
    ~ReportHeaderDialog() override = default;

    /// Установить данные шапки
    void setHeader(const ReportHeader& header);

    /// Получить данные шапки
    ReportHeader header() const;

    /// Загрузить шапку из файла проекта
    void loadFromProject(const QString& project_path);

private slots:
    void onBrowseLogo();
    void onPreviewLogo();

private:
    void setupUi();

    // Поля ввода
    QLineEdit* organization_edit_{nullptr};
    QLineEdit* contractor_edit_{nullptr};
    QLineEdit* field_edit_{nullptr};
    QLineEdit* well_pad_edit_{nullptr};
    QLineEdit* well_name_edit_{nullptr};
    QLineEdit* survey_type_edit_{nullptr};
    QDateEdit* survey_date_edit_{nullptr};
    QLineEdit* report_number_edit_{nullptr};
    QDateEdit* report_date_edit_{nullptr};
    QLineEdit* operator_edit_{nullptr};
    QLineEdit* geologist_edit_{nullptr};
    QTextEdit* notes_edit_{nullptr};
    QLineEdit* logo_path_edit_{nullptr};
    QPushButton* logo_browse_button_{nullptr};
    QLabel* logo_preview_{nullptr};
};

}  // namespace incline3d::ui

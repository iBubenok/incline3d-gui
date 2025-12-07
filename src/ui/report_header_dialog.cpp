#include "ui/report_header_dialog.h"

#include <QDateEdit>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QTextEdit>
#include <QVBoxLayout>

namespace incline3d::ui {

ReportHeaderDialog::ReportHeaderDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Шапка отчёта"));
    setMinimumSize(600, 500);
    setupUi();
}

void ReportHeaderDialog::setupUi() {
    auto* main_layout = new QVBoxLayout(this);

    // Группа организаций
    auto* org_group = new QGroupBox(tr("Организации"), this);
    auto* org_layout = new QFormLayout(org_group);

    organization_edit_ = new QLineEdit(this);
    organization_edit_->setPlaceholderText(tr("Наименование организации-заказчика"));
    org_layout->addRow(tr("Заказчик:"), organization_edit_);

    contractor_edit_ = new QLineEdit(this);
    contractor_edit_->setPlaceholderText(tr("Наименование организации-исполнителя"));
    org_layout->addRow(tr("Исполнитель:"), contractor_edit_);

    main_layout->addWidget(org_group);

    // Группа объекта
    auto* object_group = new QGroupBox(tr("Объект"), this);
    auto* object_layout = new QFormLayout(object_group);

    field_edit_ = new QLineEdit(this);
    field_edit_->setPlaceholderText(tr("Наименование месторождения"));
    object_layout->addRow(tr("Месторождение:"), field_edit_);

    well_pad_edit_ = new QLineEdit(this);
    well_pad_edit_->setPlaceholderText(tr("Номер куста"));
    object_layout->addRow(tr("Куст:"), well_pad_edit_);

    well_name_edit_ = new QLineEdit(this);
    well_name_edit_->setPlaceholderText(tr("Номер скважины"));
    object_layout->addRow(tr("Скважина:"), well_name_edit_);

    main_layout->addWidget(object_group);

    // Группа замера
    auto* survey_group = new QGroupBox(tr("Замер"), this);
    auto* survey_layout = new QFormLayout(survey_group);

    survey_type_edit_ = new QLineEdit(this);
    survey_type_edit_->setText(tr("Инклинометрия"));
    survey_layout->addRow(tr("Тип замера:"), survey_type_edit_);

    survey_date_edit_ = new QDateEdit(this);
    survey_date_edit_->setCalendarPopup(true);
    survey_date_edit_->setDate(QDate::currentDate());
    survey_layout->addRow(tr("Дата замера:"), survey_date_edit_);

    main_layout->addWidget(survey_group);

    // Группа заключения
    auto* report_group = new QGroupBox(tr("Заключение"), this);
    auto* report_layout = new QFormLayout(report_group);

    report_number_edit_ = new QLineEdit(this);
    report_number_edit_->setPlaceholderText(tr("Номер заключения"));
    report_layout->addRow(tr("№ заключения:"), report_number_edit_);

    report_date_edit_ = new QDateEdit(this);
    report_date_edit_->setCalendarPopup(true);
    report_date_edit_->setDate(QDate::currentDate());
    report_layout->addRow(tr("Дата заключения:"), report_date_edit_);

    operator_edit_ = new QLineEdit(this);
    operator_edit_->setPlaceholderText(tr("ФИО оператора"));
    report_layout->addRow(tr("Оператор:"), operator_edit_);

    geologist_edit_ = new QLineEdit(this);
    geologist_edit_->setPlaceholderText(tr("ФИО геолога"));
    report_layout->addRow(tr("Геолог:"), geologist_edit_);

    main_layout->addWidget(report_group);

    // Группа логотипа
    auto* logo_group = new QGroupBox(tr("Логотип"), this);
    auto* logo_layout = new QHBoxLayout(logo_group);

    logo_path_edit_ = new QLineEdit(this);
    logo_path_edit_->setPlaceholderText(tr("Путь к файлу логотипа"));
    logo_layout->addWidget(logo_path_edit_);

    logo_browse_button_ = new QPushButton(tr("Обзор..."), this);
    logo_layout->addWidget(logo_browse_button_);

    logo_preview_ = new QLabel(this);
    logo_preview_->setMinimumSize(64, 64);
    logo_preview_->setMaximumSize(64, 64);
    logo_preview_->setStyleSheet("QLabel { background-color: #f0f0f0; border: 1px solid #ccc; }");
    logo_preview_->setAlignment(Qt::AlignCenter);
    logo_preview_->setScaledContents(true);
    logo_layout->addWidget(logo_preview_);

    main_layout->addWidget(logo_group);

    // Примечания
    auto* notes_group = new QGroupBox(tr("Примечания"), this);
    auto* notes_layout = new QVBoxLayout(notes_group);

    notes_edit_ = new QTextEdit(this);
    notes_edit_->setMaximumHeight(80);
    notes_edit_->setPlaceholderText(tr("Дополнительные примечания..."));
    notes_layout->addWidget(notes_edit_);

    main_layout->addWidget(notes_group);

    // Кнопки
    auto* button_box = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(button_box, &QDialogButtonBox::accepted, this, &QDialog::accept);
    connect(button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    main_layout->addWidget(button_box);

    // Сигналы
    connect(logo_browse_button_, &QPushButton::clicked,
            this, &ReportHeaderDialog::onBrowseLogo);
    connect(logo_path_edit_, &QLineEdit::textChanged,
            this, &ReportHeaderDialog::onPreviewLogo);
}

void ReportHeaderDialog::setHeader(const ReportHeader& header) {
    organization_edit_->setText(header.organization);
    contractor_edit_->setText(header.contractor);
    field_edit_->setText(header.field_name);
    well_pad_edit_->setText(header.well_pad);
    well_name_edit_->setText(header.well_name);
    survey_type_edit_->setText(header.survey_type);

    if (!header.survey_date.isEmpty()) {
        survey_date_edit_->setDate(QDate::fromString(header.survey_date, Qt::ISODate));
    }

    report_number_edit_->setText(header.report_number);

    if (!header.report_date.isEmpty()) {
        report_date_edit_->setDate(QDate::fromString(header.report_date, Qt::ISODate));
    }

    operator_edit_->setText(header.operator_name);
    geologist_edit_->setText(header.geologist_name);
    notes_edit_->setPlainText(header.notes);
    logo_path_edit_->setText(header.logo_path);

    onPreviewLogo();
}

ReportHeader ReportHeaderDialog::header() const {
    ReportHeader h;
    h.organization = organization_edit_->text();
    h.contractor = contractor_edit_->text();
    h.field_name = field_edit_->text();
    h.well_pad = well_pad_edit_->text();
    h.well_name = well_name_edit_->text();
    h.survey_type = survey_type_edit_->text();
    h.survey_date = survey_date_edit_->date().toString(Qt::ISODate);
    h.report_number = report_number_edit_->text();
    h.report_date = report_date_edit_->date().toString(Qt::ISODate);
    h.operator_name = operator_edit_->text();
    h.geologist_name = geologist_edit_->text();
    h.notes = notes_edit_->toPlainText();
    h.logo_path = logo_path_edit_->text();
    return h;
}

void ReportHeaderDialog::loadFromProject(const QString& project_path) {
    Q_UNUSED(project_path)
    // TODO: загрузка шапки из файла проекта
}

void ReportHeaderDialog::onBrowseLogo() {
    QString path = QFileDialog::getOpenFileName(
        this, tr("Выбрать логотип"),
        logo_path_edit_->text(),
        tr("Изображения (*.png *.jpg *.jpeg *.bmp *.gif);;Все файлы (*)"));

    if (!path.isEmpty()) {
        logo_path_edit_->setText(path);
    }
}

void ReportHeaderDialog::onPreviewLogo() {
    QString path = logo_path_edit_->text();

    if (path.isEmpty()) {
        logo_preview_->clear();
        logo_preview_->setText(tr("Нет"));
        return;
    }

    QPixmap pixmap(path);
    if (pixmap.isNull()) {
        logo_preview_->clear();
        logo_preview_->setText(tr("Ошибка"));
    } else {
        logo_preview_->setPixmap(pixmap.scaled(
            logo_preview_->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
    }
}

}  // namespace incline3d::ui

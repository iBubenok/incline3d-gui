#include "ui/conclusion_dialog.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QDateEdit>
#include <QDesktopServices>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPrintDialog>
#include <QPrinter>
#include <QPushButton>
#include <QScrollArea>
#include <QTabWidget>
#include <QTableWidget>
#include <QTextDocument>
#include <QTextEdit>
#include <QUrl>

#include "models/well_data.h"
#include "models/project_point.h"
#include "utils/angle_utils.h"
#include "utils/logger.h"

namespace incline3d::ui {

ConclusionDialog::ConclusionDialog(
    std::shared_ptr<models::WellData> well,
    const std::vector<models::ProjectPoint>& project_points,
    QWidget* parent)
    : QDialog(parent)
    , well_(well)
    , project_points_(project_points) {
    setupUi();
    loadFromWell();
    updateSummary();
}

ConclusionDialog::~ConclusionDialog() = default;

void ConclusionDialog::setupUi() {
    setWindowTitle(tr("Формирование заключения"));
    setMinimumSize(900, 700);
    resize(1000, 800);

    auto* main_layout = new QVBoxLayout(this);

    tab_widget_ = new QTabWidget(this);
    createHeaderTab();
    createResultsTab();
    createProjectPointsTab();
    createSummaryTab();
    main_layout->addWidget(tab_widget_);

    // Настройки экспорта
    auto* export_group = new QGroupBox(tr("Настройки экспорта"), this);
    auto* export_layout = new QHBoxLayout(export_group);

    include_header_check_ = new QCheckBox(tr("Включить шапку"), export_group);
    include_header_check_->setChecked(true);
    export_layout->addWidget(include_header_check_);

    include_logo_check_ = new QCheckBox(tr("Включить логотипы"), export_group);
    include_logo_check_->setChecked(true);
    export_layout->addWidget(include_logo_check_);

    include_project_points_check_ = new QCheckBox(tr("Включить проектные точки"), export_group);
    include_project_points_check_->setChecked(true);
    export_layout->addWidget(include_project_points_check_);

    export_layout->addWidget(new QLabel(tr("Формат углов:"), export_group));
    angle_format_combo_ = new QComboBox(export_group);
    angle_format_combo_->addItem(tr("Градусы (45.50°)"), "decimal");
    angle_format_combo_->addItem(tr("Гр.мин (45.30)"), "degmin");
    angle_format_combo_->addItem(tr("Гр°мин' (45°30')"), "dms");
    export_layout->addWidget(angle_format_combo_);

    export_layout->addStretch();

    main_layout->addWidget(export_group);

    // Кнопки экспорта
    auto* buttons_layout = new QHBoxLayout();

    auto* export_csv_btn = new QPushButton(tr("Экспорт в CSV..."), this);
    connect(export_csv_btn, &QPushButton::clicked, this, &ConclusionDialog::onExportCsv);
    buttons_layout->addWidget(export_csv_btn);

    auto* export_excel_btn = new QPushButton(tr("Экспорт в Excel..."), this);
    connect(export_excel_btn, &QPushButton::clicked, this, &ConclusionDialog::onExportExcel);
    buttons_layout->addWidget(export_excel_btn);

    auto* print_btn = new QPushButton(tr("Печать..."), this);
    connect(print_btn, &QPushButton::clicked, this, &ConclusionDialog::onPrint);
    buttons_layout->addWidget(print_btn);

    auto* preview_btn = new QPushButton(tr("Предпросмотр"), this);
    connect(preview_btn, &QPushButton::clicked, this, &ConclusionDialog::onPreview);
    buttons_layout->addWidget(preview_btn);

    buttons_layout->addStretch();

    auto* close_btn = new QPushButton(tr("Закрыть"), this);
    connect(close_btn, &QPushButton::clicked, this, &QDialog::accept);
    buttons_layout->addWidget(close_btn);

    main_layout->addLayout(buttons_layout);
}

void ConclusionDialog::createHeaderTab() {
    header_tab_ = new QWidget(this);
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);

    auto* content = new QWidget();
    auto* layout = new QVBoxLayout(content);

    // Группа организации
    auto* org_group = new QGroupBox(tr("Организация"), content);
    auto* org_layout = new QFormLayout(org_group);

    company_edit_ = new QLineEdit(org_group);
    org_layout->addRow(tr("Компания:"), company_edit_);

    layout->addWidget(org_group);

    // Группа скважины
    auto* well_group = new QGroupBox(tr("Скважина"), content);
    auto* well_layout = new QFormLayout(well_group);

    field_edit_ = new QLineEdit(well_group);
    well_layout->addRow(tr("Месторождение:"), field_edit_);

    area_edit_ = new QLineEdit(well_group);
    well_layout->addRow(tr("Площадь:"), area_edit_);

    pad_edit_ = new QLineEdit(well_group);
    well_layout->addRow(tr("Куст:"), pad_edit_);

    well_edit_ = new QLineEdit(well_group);
    well_layout->addRow(tr("Скважина:"), well_edit_);

    measurement_number_edit_ = new QLineEdit(well_group);
    well_layout->addRow(tr("Номер измерения:"), measurement_number_edit_);

    date_edit_ = new QDateEdit(well_group);
    date_edit_->setCalendarPopup(true);
    date_edit_->setDisplayFormat("dd.MM.yyyy");
    date_edit_->setDate(QDate::currentDate());
    well_layout->addRow(tr("Дата:"), date_edit_);

    layout->addWidget(well_group);

    // Группа прибора
    auto* device_group = new QGroupBox(tr("Прибор"), content);
    auto* device_layout = new QFormLayout(device_group);

    device_edit_ = new QLineEdit(device_group);
    device_layout->addRow(tr("Тип прибора:"), device_edit_);

    device_number_edit_ = new QLineEdit(device_group);
    device_layout->addRow(tr("Номер прибора:"), device_number_edit_);

    calibration_edit_ = new QLineEdit(device_group);
    device_layout->addRow(tr("Поверка:"), calibration_edit_);

    layout->addWidget(device_group);

    // Группа исследования
    auto* research_group = new QGroupBox(tr("Исследование"), content);
    auto* research_layout = new QFormLayout(research_group);

    research_type_edit_ = new QLineEdit(research_group);
    research_layout->addRow(tr("Вид исследования:"), research_type_edit_);

    quality_combo_ = new QComboBox(research_group);
    quality_combo_->addItem(tr("Хорошее"), "good");
    quality_combo_->addItem(tr("Удовлетворительное"), "satisfactory");
    quality_combo_->addItem(tr("Неудовлетворительное"), "poor");
    research_layout->addRow(tr("Качество:"), quality_combo_);

    layout->addWidget(research_group);

    // Группа исполнителей
    auto* performers_group = new QGroupBox(tr("Исполнители"), content);
    auto* performers_layout = new QFormLayout(performers_group);

    operator_edit_ = new QLineEdit(performers_group);
    performers_layout->addRow(tr("Оператор:"), operator_edit_);

    interpreter_edit_ = new QLineEdit(performers_group);
    performers_layout->addRow(tr("Интерпретатор:"), interpreter_edit_);

    party_chief_edit_ = new QLineEdit(performers_group);
    performers_layout->addRow(tr("Начальник партии:"), party_chief_edit_);

    customer_edit_ = new QLineEdit(performers_group);
    performers_layout->addRow(tr("Заказчик:"), customer_edit_);

    layout->addWidget(performers_group);

    // Группа логотипов
    auto* logo_group = new QGroupBox(tr("Логотипы"), content);
    auto* logo_layout = new QFormLayout(logo_group);

    auto* logo_left_layout = new QHBoxLayout();
    logo_left_edit_ = new QLineEdit(logo_group);
    logo_left_edit_->setReadOnly(true);
    logo_left_layout->addWidget(logo_left_edit_);
    auto* browse_left_btn = new QPushButton(tr("..."), logo_group);
    browse_left_btn->setMaximumWidth(30);
    connect(browse_left_btn, &QPushButton::clicked, this, &ConclusionDialog::onSelectLogoLeft);
    logo_left_layout->addWidget(browse_left_btn);
    auto* clear_left_btn = new QPushButton(tr("X"), logo_group);
    clear_left_btn->setMaximumWidth(30);
    connect(clear_left_btn, &QPushButton::clicked, this, &ConclusionDialog::onClearLogoLeft);
    logo_left_layout->addWidget(clear_left_btn);
    logo_layout->addRow(tr("Левый логотип:"), logo_left_layout);

    auto* logo_right_layout = new QHBoxLayout();
    logo_right_edit_ = new QLineEdit(logo_group);
    logo_right_edit_->setReadOnly(true);
    logo_right_layout->addWidget(logo_right_edit_);
    auto* browse_right_btn = new QPushButton(tr("..."), logo_group);
    browse_right_btn->setMaximumWidth(30);
    connect(browse_right_btn, &QPushButton::clicked, this, &ConclusionDialog::onSelectLogoRight);
    logo_right_layout->addWidget(browse_right_btn);
    auto* clear_right_btn = new QPushButton(tr("X"), logo_group);
    clear_right_btn->setMaximumWidth(30);
    connect(clear_right_btn, &QPushButton::clicked, this, &ConclusionDialog::onClearLogoRight);
    logo_right_layout->addWidget(clear_right_btn);
    logo_layout->addRow(tr("Правый логотип:"), logo_right_layout);

    layout->addWidget(logo_group);

    layout->addStretch();
    scroll->setWidget(content);

    auto* tab_layout = new QVBoxLayout(header_tab_);
    tab_layout->setContentsMargins(0, 0, 0, 0);
    tab_layout->addWidget(scroll);

    tab_widget_->addTab(header_tab_, tr("Шапка"));
}

void ConclusionDialog::createResultsTab() {
    results_tab_ = new QWidget(this);
    auto* layout = new QVBoxLayout(results_tab_);

    results_table_ = new QTableWidget(results_tab_);
    results_table_->setColumnCount(12);
    results_table_->setHorizontalHeaderLabels({
        tr("Глубина"), tr("Угол"), tr("Азимут"), tr("Азимут пр."),
        tr("Север"), tr("Восток"), tr("TVD"), tr("Инт. 10м"),
        tr("Инт. L"), tr("Ош. X"), tr("Ош. Y"), tr("Ош. Z")
    });
    results_table_->horizontalHeader()->setStretchLastSection(true);
    results_table_->setAlternatingRowColors(true);
    results_table_->setEditTriggers(QAbstractItemView::NoEditTriggers);

    layout->addWidget(results_table_);

    tab_widget_->addTab(results_tab_, tr("Результаты"));
}

void ConclusionDialog::createProjectPointsTab() {
    project_tab_ = new QWidget(this);
    auto* layout = new QVBoxLayout(project_tab_);

    project_table_ = new QTableWidget(project_tab_);
    project_table_->setColumnCount(10);
    project_table_->setHorizontalHeaderLabels({
        tr("Пласт"), tr("Азимут план"), tr("Смещение план"), tr("Глубина план"),
        tr("Угол факт"), tr("Азимут факт"), tr("Север факт"), tr("Восток факт"),
        tr("Смещение факт"), tr("Радиус допуска")
    });
    project_table_->horizontalHeader()->setStretchLastSection(true);
    project_table_->setAlternatingRowColors(true);
    project_table_->setEditTriggers(QAbstractItemView::NoEditTriggers);

    layout->addWidget(project_table_);

    tab_widget_->addTab(project_tab_, tr("Проектные точки"));
}

void ConclusionDialog::createSummaryTab() {
    summary_tab_ = new QWidget(this);
    auto* layout = new QVBoxLayout(summary_tab_);

    summary_text_ = new QTextEdit(summary_tab_);
    summary_text_->setReadOnly(true);
    layout->addWidget(summary_text_);

    tab_widget_->addTab(summary_tab_, tr("Сводка"));
}

void ConclusionDialog::loadFromWell() {
    if (!well_) return;

    const auto& meta = well_->metadata;

    // Заполнение шапки из метаданных скважины
    field_edit_->setText(QString::fromStdString(meta.field_name));
    area_edit_->setText(QString::fromStdString(meta.area));
    pad_edit_->setText(QString::fromStdString(meta.well_pad));
    well_edit_->setText(QString::fromStdString(meta.well_name));
    measurement_number_edit_->setText(QString::fromStdString(meta.measurement_number));
    device_edit_->setText(QString::fromStdString(meta.device));
    device_number_edit_->setText(QString::fromStdString(meta.device_number));
    calibration_edit_->setText(QString::fromStdString(meta.device_calibration_date));
    research_type_edit_->setText(QString::fromStdString(meta.research_type));
    interpreter_edit_->setText(QString::fromStdString(meta.interpreter));
    party_chief_edit_->setText(QString::fromStdString(meta.party_chief));
    customer_edit_->setText(QString::fromStdString(meta.customer));

    if (!meta.research_date.empty()) {
        date_edit_->setDate(QDate::fromString(QString::fromStdString(meta.research_date), "dd.MM.yyyy"));
    }

    // Качество
    if (meta.quality == "good") {
        quality_combo_->setCurrentIndex(0);
    } else if (meta.quality == "satisfactory") {
        quality_combo_->setCurrentIndex(1);
    } else if (meta.quality == "poor") {
        quality_combo_->setCurrentIndex(2);
    }

    // Заполнение таблицы результатов
    results_table_->setRowCount(well_->results.size());
    for (size_t i = 0; i < well_->results.size(); ++i) {
        const auto& pt = well_->results[i];
        int row = static_cast<int>(i);

        results_table_->setItem(row, 0, new QTableWidgetItem(QString::number(pt.measured_depth_m, 'f', 2)));
        results_table_->setItem(row, 1, new QTableWidgetItem(QString::number(pt.inclination_deg, 'f', 2)));
        results_table_->setItem(row, 2, new QTableWidgetItem(
            pt.azimuth_deg.has_value() ? QString::number(pt.azimuth_deg.value(), 'f', 2) : "-"));
        results_table_->setItem(row, 3, new QTableWidgetItem(QString::number(pt.applied_azimuth_deg, 'f', 2)));
        results_table_->setItem(row, 4, new QTableWidgetItem(QString::number(pt.north_m, 'f', 2)));
        results_table_->setItem(row, 5, new QTableWidgetItem(QString::number(pt.east_m, 'f', 2)));
        results_table_->setItem(row, 6, new QTableWidgetItem(QString::number(pt.tvd_m, 'f', 2)));
        results_table_->setItem(row, 7, new QTableWidgetItem(QString::number(pt.intensity_10m, 'f', 2)));
        results_table_->setItem(row, 8, new QTableWidgetItem(QString::number(pt.intensity_L, 'f', 2)));
        results_table_->setItem(row, 9, new QTableWidgetItem(QString::number(pt.mistake_x, 'f', 3)));
        results_table_->setItem(row, 10, new QTableWidgetItem(QString::number(pt.mistake_y, 'f', 3)));
        results_table_->setItem(row, 11, new QTableWidgetItem(QString::number(pt.mistake_z, 'f', 3)));
    }

    // Заполнение таблицы проектных точек
    project_table_->setRowCount(project_points_.size());
    for (size_t i = 0; i < project_points_.size(); ++i) {
        const auto& pt = project_points_[i];
        int row = static_cast<int>(i);

        project_table_->setItem(row, 0, new QTableWidgetItem(QString::fromStdString(pt.name)));
        project_table_->setItem(row, 1, new QTableWidgetItem(QString::number(pt.azimuth_geogr_deg, 'f', 2)));
        project_table_->setItem(row, 2, new QTableWidgetItem(QString::number(pt.shift_m, 'f', 2)));
        project_table_->setItem(row, 3, new QTableWidgetItem(QString::number(pt.depth_m, 'f', 2)));
        project_table_->setItem(row, 4, new QTableWidgetItem(QString::number(pt.fact_inclination_deg, 'f', 2)));
        project_table_->setItem(row, 5, new QTableWidgetItem(QString::number(pt.fact_azimuth_deg, 'f', 2)));
        project_table_->setItem(row, 6, new QTableWidgetItem(QString::number(pt.fact_north_m, 'f', 2)));
        project_table_->setItem(row, 7, new QTableWidgetItem(QString::number(pt.fact_east_m, 'f', 2)));
        project_table_->setItem(row, 8, new QTableWidgetItem(QString::number(pt.fact_offset_m, 'f', 2)));
        project_table_->setItem(row, 9, new QTableWidgetItem(QString::number(pt.radius_m, 'f', 2)));
    }
}

void ConclusionDialog::updateSummary() {
    if (!well_) return;

    QString summary;
    summary += tr("=== СВОДКА ПО РЕЗУЛЬТАТАМ ИНКЛИНОМЕТРИИ ===\n\n");

    summary += tr("Скважина: %1\n").arg(well_edit_->text());
    summary += tr("Месторождение: %1\n").arg(field_edit_->text());
    summary += tr("Куст: %1\n").arg(pad_edit_->text());
    summary += tr("Дата: %1\n\n").arg(date_edit_->date().toString("dd.MM.yyyy"));

    if (!well_->results.empty()) {
        const auto& first = well_->results.front();
        const auto& last = well_->results.back();

        summary += tr("Интервал измерений: %.2f - %.2f м\n")
            .arg(first.measured_depth_m)
            .arg(last.measured_depth_m);
        summary += tr("Количество точек: %1\n\n").arg(well_->results.size());

        summary += tr("Забойные координаты:\n");
        summary += tr("  TVD: %.2f м\n").arg(last.tvd_m);
        summary += tr("  Север: %.2f м\n").arg(last.north_m);
        summary += tr("  Восток: %.2f м\n").arg(last.east_m);
        summary += tr("  Горизонтальное смещение: %.2f м\n\n")
            .arg(std::sqrt(last.north_m * last.north_m + last.east_m * last.east_m));

        summary += tr("Максимальный угол: %.2f° (на глубине %.2f м)\n")
            .arg(well_->max_inclination_deg)
            .arg(well_->total_depth);
        summary += tr("Максимальная интенсивность (10м): %.2f°/10м (на глубине %.2f м)\n")
            .arg(well_->max_intensity_10m)
            .arg(well_->max_intensity_10m_depth);
        summary += tr("Максимальная интенсивность (L): %.2f°/L (на глубине %.2f м)\n\n")
            .arg(well_->max_intensity_L)
            .arg(well_->max_intensity_L_depth);
    }

    if (!project_points_.empty()) {
        summary += tr("Проектные точки: %1\n").arg(project_points_.size());
        for (const auto& pt : project_points_) {
            double deviation = std::sqrt(
                std::pow(pt.fact_north_m - pt.shift_m * std::cos(pt.azimuth_geogr_deg * M_PI / 180.0), 2) +
                std::pow(pt.fact_east_m - pt.shift_m * std::sin(pt.azimuth_geogr_deg * M_PI / 180.0), 2));

            QString status = (deviation <= pt.radius_m) ? tr("В допуске") : tr("ВЫХОД ИЗ ДОПУСКА");
            summary += tr("  %1: отклонение %.2f м (%2)\n")
                .arg(QString::fromStdString(pt.name))
                .arg(deviation)
                .arg(status);
        }
    }

    summary_text_->setPlainText(summary);
}

ConclusionHeader ConclusionDialog::header() const {
    ConclusionHeader h;
    h.company = company_edit_->text();
    h.field = field_edit_->text();
    h.area = area_edit_->text();
    h.pad = pad_edit_->text();
    h.well = well_edit_->text();
    h.measurement_number = measurement_number_edit_->text();
    h.date = date_edit_->date().toString("dd.MM.yyyy");
    h.device = device_edit_->text();
    h.device_number = device_number_edit_->text();
    h.device_calibration = calibration_edit_->text();
    h.research_type = research_type_edit_->text();
    h.quality = quality_combo_->currentData().toString();
    h.operator_name = operator_edit_->text();
    h.interpreter = interpreter_edit_->text();
    h.party_chief = party_chief_edit_->text();
    h.customer = customer_edit_->text();
    h.logo_left = logo_left_edit_->text();
    h.logo_right = logo_right_edit_->text();
    return h;
}

void ConclusionDialog::setHeader(const ConclusionHeader& h) {
    company_edit_->setText(h.company);
    field_edit_->setText(h.field);
    area_edit_->setText(h.area);
    pad_edit_->setText(h.pad);
    well_edit_->setText(h.well);
    measurement_number_edit_->setText(h.measurement_number);
    date_edit_->setDate(QDate::fromString(h.date, "dd.MM.yyyy"));
    device_edit_->setText(h.device);
    device_number_edit_->setText(h.device_number);
    calibration_edit_->setText(h.device_calibration);
    research_type_edit_->setText(h.research_type);
    operator_edit_->setText(h.operator_name);
    interpreter_edit_->setText(h.interpreter);
    party_chief_edit_->setText(h.party_chief);
    customer_edit_->setText(h.customer);
    logo_left_edit_->setText(h.logo_left);
    logo_right_edit_->setText(h.logo_right);
}

QString ConclusionDialog::generateCsvContent() const {
    QString csv;
    QString sep = ";";

    if (include_header_check_->isChecked()) {
        csv += tr("Месторождение") + sep + field_edit_->text() + "\n";
        csv += tr("Площадь") + sep + area_edit_->text() + "\n";
        csv += tr("Куст") + sep + pad_edit_->text() + "\n";
        csv += tr("Скважина") + sep + well_edit_->text() + "\n";
        csv += tr("Дата") + sep + date_edit_->date().toString("dd.MM.yyyy") + "\n";
        csv += "\n";
    }

    // Заголовки результатов
    csv += tr("Глубина") + sep + tr("Угол") + sep + tr("Азимут") + sep + tr("Азимут пр.") + sep;
    csv += tr("Север") + sep + tr("Восток") + sep + tr("TVD") + sep;
    csv += tr("Инт. 10м") + sep + tr("Инт. L") + "\n";

    // Данные результатов
    for (const auto& pt : well_->results) {
        csv += QString::number(pt.measured_depth_m, 'f', 2) + sep;
        csv += QString::number(pt.inclination_deg, 'f', 2) + sep;
        csv += (pt.azimuth_deg.has_value() ? QString::number(pt.azimuth_deg.value(), 'f', 2) : "") + sep;
        csv += QString::number(pt.applied_azimuth_deg, 'f', 2) + sep;
        csv += QString::number(pt.north_m, 'f', 2) + sep;
        csv += QString::number(pt.east_m, 'f', 2) + sep;
        csv += QString::number(pt.tvd_m, 'f', 2) + sep;
        csv += QString::number(pt.intensity_10m, 'f', 2) + sep;
        csv += QString::number(pt.intensity_L, 'f', 2) + "\n";
    }

    if (include_project_points_check_->isChecked() && !project_points_.empty()) {
        csv += "\n" + tr("ПРОЕКТНЫЕ ТОЧКИ") + "\n";
        csv += tr("Пласт") + sep + tr("Азимут план") + sep + tr("Смещение план") + sep;
        csv += tr("Глубина план") + sep + tr("Смещение факт") + sep + tr("Радиус допуска") + "\n";

        for (const auto& pt : project_points_) {
            csv += QString::fromStdString(pt.name) + sep;
            csv += QString::number(pt.azimuth_geogr_deg, 'f', 2) + sep;
            csv += QString::number(pt.shift_m, 'f', 2) + sep;
            csv += QString::number(pt.depth_m, 'f', 2) + sep;
            csv += QString::number(pt.fact_offset_m, 'f', 2) + sep;
            csv += QString::number(pt.radius_m, 'f', 2) + "\n";
        }
    }

    return csv;
}

QString ConclusionDialog::generateHtmlReport() const {
    QString html = "<!DOCTYPE html><html><head><meta charset='utf-8'>";
    html += "<style>";
    html += "body { font-family: Arial, sans-serif; margin: 20px; }";
    html += "h1 { text-align: center; }";
    html += "table { border-collapse: collapse; width: 100%; margin: 10px 0; }";
    html += "th, td { border: 1px solid #333; padding: 5px; text-align: center; }";
    html += "th { background-color: #e0e0e0; }";
    html += ".header { margin-bottom: 20px; }";
    html += ".logos { display: flex; justify-content: space-between; margin-bottom: 10px; }";
    html += ".logo { max-width: 150px; max-height: 80px; }";
    html += "</style></head><body>";

    if (include_header_check_->isChecked()) {
        html += "<div class='header'>";

        if (include_logo_check_->isChecked()) {
            html += "<div class='logos'>";
            if (!logo_left_edit_->text().isEmpty()) {
                html += QString("<img class='logo' src='%1'>").arg(logo_left_edit_->text());
            }
            html += QString("<h1>%1</h1>").arg(tr("ЗАКЛЮЧЕНИЕ ПО ИНКЛИНОМЕТРИИ"));
            if (!logo_right_edit_->text().isEmpty()) {
                html += QString("<img class='logo' src='%1'>").arg(logo_right_edit_->text());
            }
            html += "</div>";
        } else {
            html += QString("<h1>%1</h1>").arg(tr("ЗАКЛЮЧЕНИЕ ПО ИНКЛИНОМЕТРИИ"));
        }

        html += "<table>";
        html += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td></tr>")
            .arg(tr("Месторождение")).arg(field_edit_->text())
            .arg(tr("Площадь")).arg(area_edit_->text());
        html += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td></tr>")
            .arg(tr("Куст")).arg(pad_edit_->text())
            .arg(tr("Скважина")).arg(well_edit_->text());
        html += QString("<tr><td>%1</td><td>%2</td><td>%3</td><td>%4</td></tr>")
            .arg(tr("Дата")).arg(date_edit_->date().toString("dd.MM.yyyy"))
            .arg(tr("Прибор")).arg(device_edit_->text());
        html += "</table>";
        html += "</div>";
    }

    // Таблица результатов
    html += QString("<h2>%1</h2>").arg(tr("Результаты обработки"));
    html += "<table><tr>";
    html += QString("<th>%1</th><th>%2</th><th>%3</th><th>%4</th>")
        .arg(tr("Глубина")).arg(tr("Угол")).arg(tr("Азимут")).arg(tr("Азимут пр."));
    html += QString("<th>%1</th><th>%2</th><th>%3</th><th>%4</th><th>%5</th></tr>")
        .arg(tr("Север")).arg(tr("Восток")).arg(tr("TVD")).arg(tr("Инт. 10м")).arg(tr("Инт. L"));

    for (const auto& pt : well_->results) {
        html += "<tr>";
        html += QString("<td>%1</td>").arg(pt.measured_depth_m, 0, 'f', 2);
        html += QString("<td>%1</td>").arg(pt.inclination_deg, 0, 'f', 2);
        html += QString("<td>%1</td>").arg(pt.azimuth_deg.has_value() ? QString::number(pt.azimuth_deg.value(), 'f', 2) : "-");
        html += QString("<td>%1</td>").arg(pt.applied_azimuth_deg, 0, 'f', 2);
        html += QString("<td>%1</td>").arg(pt.north_m, 0, 'f', 2);
        html += QString("<td>%1</td>").arg(pt.east_m, 0, 'f', 2);
        html += QString("<td>%1</td>").arg(pt.tvd_m, 0, 'f', 2);
        html += QString("<td>%1</td>").arg(pt.intensity_10m, 0, 'f', 2);
        html += QString("<td>%1</td>").arg(pt.intensity_L, 0, 'f', 2);
        html += "</tr>";
    }
    html += "</table>";

    html += "</body></html>";
    return html;
}

void ConclusionDialog::onExportCsv() {
    QString path = QFileDialog::getSaveFileName(
        this, tr("Экспорт в CSV"),
        QString(),
        tr("CSV файлы (*.csv);;Все файлы (*)"));

    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Ошибка"),
                              tr("Не удалось создать файл"));
        return;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << generateCsvContent();
    file.close();

    LOG_INFO(tr("Заключение экспортировано в CSV: %1").arg(path));
    QMessageBox::information(this, tr("Экспорт"),
                             tr("Заключение сохранено в файл:\n%1").arg(path));
}

void ConclusionDialog::onExportExcel() {
    // Экспорт в формат, совместимый с Excel (CSV с BOM для правильной кодировки)
    QString path = QFileDialog::getSaveFileName(
        this, tr("Экспорт для Excel"),
        QString(),
        tr("CSV файлы (*.csv);;Все файлы (*)"));

    if (path.isEmpty()) return;

    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QMessageBox::critical(this, tr("Ошибка"),
                              tr("Не удалось создать файл"));
        return;
    }

    // Запись BOM для UTF-8
    file.write("\xEF\xBB\xBF");

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);
    out << generateCsvContent();
    file.close();

    LOG_INFO(tr("Заключение экспортировано для Excel: %1").arg(path));
    QMessageBox::information(this, tr("Экспорт"),
                             tr("Заключение сохранено для Excel:\n%1").arg(path));
}

void ConclusionDialog::onPrint() {
    QPrinter printer(QPrinter::HighResolution);
    QPrintDialog dialog(&printer, this);

    if (dialog.exec() == QDialog::Accepted) {
        QTextDocument doc;
        doc.setHtml(generateHtmlReport());
        doc.print(&printer);
        LOG_INFO(tr("Заключение отправлено на печать"));
    }
}

void ConclusionDialog::onPreview() {
    // Сохранение во временный HTML и открытие в браузере
    QString temp_path = QDir::temp().filePath("incline3d_conclusion_preview.html");

    QFile file(temp_path);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        out.setEncoding(QStringConverter::Utf8);
        out << generateHtmlReport();
        file.close();

        QDesktopServices::openUrl(QUrl::fromLocalFile(temp_path));
    }
}

void ConclusionDialog::onSelectLogoLeft() {
    QString path = QFileDialog::getOpenFileName(
        this, tr("Выбрать логотип"),
        QString(),
        tr("Изображения (*.png *.jpg *.jpeg *.bmp);;Все файлы (*)"));
    if (!path.isEmpty()) {
        logo_left_edit_->setText(path);
    }
}

void ConclusionDialog::onSelectLogoRight() {
    QString path = QFileDialog::getOpenFileName(
        this, tr("Выбрать логотип"),
        QString(),
        tr("Изображения (*.png *.jpg *.jpeg *.bmp);;Все файлы (*)"));
    if (!path.isEmpty()) {
        logo_right_edit_->setText(path);
    }
}

void ConclusionDialog::onClearLogoLeft() {
    logo_left_edit_->clear();
}

void ConclusionDialog::onClearLogoRight() {
    logo_right_edit_->clear();
}

}  // namespace incline3d::ui

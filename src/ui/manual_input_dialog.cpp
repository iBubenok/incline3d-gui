#include "ui/manual_input_dialog.h"

#include <QApplication>
#include <QBoxLayout>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QDateEdit>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScrollArea>
#include <QSplitter>
#include <QTabWidget>
#include <QTableView>
#include <QTimer>
#include <QToolBar>

#include "models/measurements_model.h"
#include "utils/angle_utils.h"
#include "utils/logger.h"

namespace incline3d::ui {

ManualInputDialog::ManualInputDialog(QWidget* parent)
    : QDialog(parent) {
    well_ = std::make_shared<models::WellData>();
    measurements_model_ = std::make_unique<models::MeasurementsModel>(this);
    setupUi();
}

ManualInputDialog::ManualInputDialog(std::shared_ptr<models::WellData> well, QWidget* parent)
    : QDialog(parent)
    , well_(well) {
    if (!well_) {
        well_ = std::make_shared<models::WellData>();
    }
    measurements_model_ = std::make_unique<models::MeasurementsModel>(this);
    measurements_model_->setWell(well_);
    setupUi();
    loadFromWell();
}

ManualInputDialog::~ManualInputDialog() = default;

void ManualInputDialog::setupUi() {
    setWindowTitle(tr("Ручной ввод исходных данных"));
    setMinimumSize(900, 700);
    resize(1000, 750);

    auto* main_layout = new QVBoxLayout(this);

    tab_widget_ = new QTabWidget(this);
    createFieldsTab();
    createArrayTab();
    main_layout->addWidget(tab_widget_);

    // Панель диагностики
    auto* diag_layout = new QHBoxLayout();
    error_label_ = new QLabel(this);
    error_label_->setStyleSheet("color: red;");
    diag_layout->addWidget(error_label_);
    diag_layout->addStretch();
    points_count_label_ = new QLabel(tr("Точек: 0"), this);
    diag_layout->addWidget(points_count_label_);
    main_layout->addLayout(diag_layout);

    // Кнопки
    auto* button_box = new QDialogButtonBox(
        QDialogButtonBox::Save | QDialogButtonBox::Cancel, this);
    connect(button_box, &QDialogButtonBox::accepted, this, &ManualInputDialog::onSave);
    connect(button_box, &QDialogButtonBox::rejected, this, &ManualInputDialog::onCancel);
    main_layout->addWidget(button_box);

    // Обновление счётчика точек
    connect(measurements_model_.get(), &QAbstractItemModel::rowsInserted,
            this, [this]() {
                points_count_label_->setText(tr("Точек: %1").arg(measurements_model_->rowCount()));
            });
    connect(measurements_model_.get(), &QAbstractItemModel::rowsRemoved,
            this, [this]() {
                points_count_label_->setText(tr("Точек: %1").arg(measurements_model_->rowCount()));
            });
}

void ManualInputDialog::createFieldsTab() {
    fields_tab_ = new QWidget(this);
    auto* scroll = new QScrollArea(this);
    scroll->setWidgetResizable(true);

    auto* content = new QWidget();
    auto* layout = new QVBoxLayout(content);

    // Группа идентификации скважины
    auto* id_group = new QGroupBox(tr("Идентификация скважины"), content);
    auto* id_layout = new QFormLayout(id_group);

    uwi_edit_ = new QLineEdit(id_group);
    id_layout->addRow(tr("UWI:"), uwi_edit_);

    region_edit_ = new QLineEdit(id_group);
    id_layout->addRow(tr("Регион:"), region_edit_);

    field_edit_ = new QLineEdit(id_group);
    id_layout->addRow(tr("Месторождение:"), field_edit_);

    area_edit_ = new QLineEdit(id_group);
    id_layout->addRow(tr("Площадь:"), area_edit_);

    pad_edit_ = new QLineEdit(id_group);
    id_layout->addRow(tr("Куст:"), pad_edit_);

    well_edit_ = new QLineEdit(id_group);
    id_layout->addRow(tr("Скважина:"), well_edit_);

    measurement_number_edit_ = new QLineEdit(id_group);
    id_layout->addRow(tr("Номер измерения:"), measurement_number_edit_);

    layout->addWidget(id_group);

    // Группа прибора
    auto* device_group = new QGroupBox(tr("Прибор"), content);
    auto* device_layout = new QFormLayout(device_group);

    device_edit_ = new QLineEdit(device_group);
    device_layout->addRow(tr("Тип прибора:"), device_edit_);

    device_number_edit_ = new QLineEdit(device_group);
    device_layout->addRow(tr("Номер прибора:"), device_number_edit_);

    device_calibration_date_ = new QDateEdit(device_group);
    device_calibration_date_->setCalendarPopup(true);
    device_calibration_date_->setDisplayFormat("dd.MM.yyyy");
    device_layout->addRow(tr("Дата поверки:"), device_calibration_date_);

    layout->addWidget(device_group);

    // Группа интервала и параметров
    auto* interval_group = new QGroupBox(tr("Интервал и параметры"), content);
    auto* interval_layout = new QFormLayout(interval_group);

    interval_start_spin_ = new QDoubleSpinBox(interval_group);
    interval_start_spin_->setRange(0, 100000);
    interval_start_spin_->setDecimals(2);
    interval_start_spin_->setSuffix(tr(" м"));
    interval_layout->addRow(tr("Начало интервала:"), interval_start_spin_);

    interval_end_spin_ = new QDoubleSpinBox(interval_group);
    interval_end_spin_->setRange(0, 100000);
    interval_end_spin_->setDecimals(2);
    interval_end_spin_->setSuffix(tr(" м"));
    interval_layout->addRow(tr("Конец интервала:"), interval_end_spin_);

    mag_declination_spin_ = new QDoubleSpinBox(interval_group);
    mag_declination_spin_->setRange(-180, 180);
    mag_declination_spin_->setDecimals(2);
    mag_declination_spin_->setSuffix(tr("°"));
    mag_declination_spin_->setToolTip(tr("Магнитное склонение или суммарная поправка (магн. склонение − сближение меридианов)"));
    interval_layout->addRow(tr("Магн. склонение:"), mag_declination_spin_);

    kelly_bushing_spin_ = new QDoubleSpinBox(interval_group);
    kelly_bushing_spin_->setRange(-1000, 10000);
    kelly_bushing_spin_->setDecimals(2);
    kelly_bushing_spin_->setSuffix(tr(" м"));
    interval_layout->addRow(tr("Альтитуда стола ротора:"), kelly_bushing_spin_);

    casing_shoe_spin_ = new QDoubleSpinBox(interval_group);
    casing_shoe_spin_->setRange(0, 10000);
    casing_shoe_spin_->setDecimals(2);
    casing_shoe_spin_->setSuffix(tr(" м"));
    interval_layout->addRow(tr("Башмак кондуктора:"), casing_shoe_spin_);

    layout->addWidget(interval_group);

    // Группа параметров скважины
    auto* well_params_group = new QGroupBox(tr("Параметры скважины"), content);
    auto* well_params_layout = new QFormLayout(well_params_group);

    d_casing_spin_ = new QDoubleSpinBox(well_params_group);
    d_casing_spin_->setRange(0, 1000);
    d_casing_spin_->setDecimals(1);
    d_casing_spin_->setSuffix(tr(" мм"));
    well_params_layout->addRow(tr("Диаметр скважины:"), d_casing_spin_);

    d_collar_spin_ = new QDoubleSpinBox(well_params_group);
    d_collar_spin_->setRange(0, 1000);
    d_collar_spin_->setDecimals(1);
    d_collar_spin_->setSuffix(tr(" мм"));
    well_params_layout->addRow(tr("Диаметр колонны:"), d_collar_spin_);

    current_depth_spin_ = new QDoubleSpinBox(well_params_group);
    current_depth_spin_->setRange(0, 100000);
    current_depth_spin_->setDecimals(2);
    current_depth_spin_->setSuffix(tr(" м"));
    well_params_layout->addRow(tr("Текущий забой:"), current_depth_spin_);

    project_depth_spin_ = new QDoubleSpinBox(well_params_group);
    project_depth_spin_->setRange(0, 100000);
    project_depth_spin_->setDecimals(2);
    project_depth_spin_->setSuffix(tr(" м"));
    well_params_layout->addRow(tr("Проектный забой:"), project_depth_spin_);

    layout->addWidget(well_params_group);

    // Группа проектных параметров забоя
    auto* project_group = new QGroupBox(tr("Проектные параметры забоя"), content);
    auto* project_layout = new QFormLayout(project_group);

    project_shift_spin_ = new QDoubleSpinBox(project_group);
    project_shift_spin_->setRange(0, 10000);
    project_shift_spin_->setDecimals(2);
    project_shift_spin_->setSuffix(tr(" м"));
    project_layout->addRow(tr("Проектное смещение:"), project_shift_spin_);

    project_azimuth_spin_ = new QDoubleSpinBox(project_group);
    project_azimuth_spin_->setRange(0, 360);
    project_azimuth_spin_->setDecimals(2);
    project_azimuth_spin_->setSuffix(tr("°"));
    project_layout->addRow(tr("Проектный азимут:"), project_azimuth_spin_);

    tolerance_radius_spin_ = new QDoubleSpinBox(project_group);
    tolerance_radius_spin_->setRange(0, 1000);
    tolerance_radius_spin_->setDecimals(2);
    tolerance_radius_spin_->setSuffix(tr(" м"));
    tolerance_radius_spin_->setToolTip(tr("Радиус круга допуска для забоя"));
    project_layout->addRow(tr("Допустимый отход:"), tolerance_radius_spin_);

    layout->addWidget(project_group);

    // Группа погрешностей
    auto* errors_group = new QGroupBox(tr("Погрешности измерений"), content);
    auto* errors_layout = new QFormLayout(errors_group);

    angle_error_spin_ = new QDoubleSpinBox(errors_group);
    angle_error_spin_->setRange(0, 10);
    angle_error_spin_->setDecimals(2);
    angle_error_spin_->setSuffix(tr("°"));
    angle_error_spin_->setValue(0.1);
    errors_layout->addRow(tr("Погрешность угла:"), angle_error_spin_);

    azimuth_error_spin_ = new QDoubleSpinBox(errors_group);
    azimuth_error_spin_->setRange(0, 10);
    azimuth_error_spin_->setDecimals(2);
    azimuth_error_spin_->setSuffix(tr("°"));
    azimuth_error_spin_->setValue(0.1);
    errors_layout->addRow(tr("Погрешность азимута:"), azimuth_error_spin_);

    layout->addWidget(errors_group);

    // Группа организационных данных
    auto* org_group = new QGroupBox(tr("Организационные данные"), content);
    auto* org_layout = new QFormLayout(org_group);

    research_date_ = new QDateEdit(org_group);
    research_date_->setCalendarPopup(true);
    research_date_->setDisplayFormat("dd.MM.yyyy");
    research_date_->setDate(QDate::currentDate());
    org_layout->addRow(tr("Дата исследования:"), research_date_);

    conditions_edit_ = new QLineEdit(org_group);
    org_layout->addRow(tr("Условия исследования:"), conditions_edit_);

    research_type_edit_ = new QLineEdit(org_group);
    org_layout->addRow(tr("Характер исследования:"), research_type_edit_);

    quality_combo_ = new QComboBox(org_group);
    quality_combo_->addItem(tr("Хорошее"), "good");
    quality_combo_->addItem(tr("Удовлетворительное"), "satisfactory");
    quality_combo_->addItem(tr("Неудовлетворительное"), "poor");
    org_layout->addRow(tr("Качество измерения:"), quality_combo_);

    customer_edit_ = new QLineEdit(org_group);
    org_layout->addRow(tr("Заказчик:"), customer_edit_);

    contractor_edit_ = new QLineEdit(org_group);
    org_layout->addRow(tr("Подрядчик:"), contractor_edit_);

    interpreter_edit_ = new QLineEdit(org_group);
    org_layout->addRow(tr("Интерпретатор:"), interpreter_edit_);

    party_chief_edit_ = new QLineEdit(org_group);
    org_layout->addRow(tr("Начальник партии:"), party_chief_edit_);

    layout->addWidget(org_group);

    layout->addStretch();

    scroll->setWidget(content);
    auto* tab_layout = new QVBoxLayout(fields_tab_);
    tab_layout->setContentsMargins(0, 0, 0, 0);
    tab_layout->addWidget(scroll);

    tab_widget_->addTab(fields_tab_, tr("Поля"));
}

void ManualInputDialog::createArrayTab() {
    array_tab_ = new QWidget(this);
    auto* layout = new QVBoxLayout(array_tab_);

    // Панель инструментов
    auto* toolbar = new QToolBar(array_tab_);

    add_row_btn_ = new QPushButton(tr("Добавить строку"), toolbar);
    connect(add_row_btn_, &QPushButton::clicked, this, &ManualInputDialog::onAddRow);
    toolbar->addWidget(add_row_btn_);

    remove_row_btn_ = new QPushButton(tr("Удалить строку"), toolbar);
    connect(remove_row_btn_, &QPushButton::clicked, this, &ManualInputDialog::onRemoveRow);
    toolbar->addWidget(remove_row_btn_);

    toolbar->addSeparator();

    import_clipboard_btn_ = new QPushButton(tr("Вставить из буфера"), toolbar);
    import_clipboard_btn_->setToolTip(tr("Вставить данные из буфера обмена (Excel, текст)"));
    connect(import_clipboard_btn_, &QPushButton::clicked, this, &ManualInputDialog::onImportFromClipboard);
    toolbar->addWidget(import_clipboard_btn_);

    toolbar->addSeparator();

    flip_array_btn_ = new QPushButton(tr("Перевернуть массив"), toolbar);
    flip_array_btn_->setToolTip(tr("Перевернуть порядок строк (для глубин по убыванию)"));
    connect(flip_array_btn_, &QPushButton::clicked, this, &ManualInputDialog::onFlipArray);
    toolbar->addWidget(flip_array_btn_);

    flip_column_btn_ = new QPushButton(tr("Перевернуть колонку"), toolbar);
    flip_column_btn_->setToolTip(tr("Перевернуть значения в выбранной колонке"));
    connect(flip_column_btn_, &QPushButton::clicked, this, &ManualInputDialog::onFlipColumn);
    toolbar->addWidget(flip_column_btn_);

    replace_empty_btn_ = new QPushButton(tr("Пустые значения..."), toolbar);
    replace_empty_btn_->setToolTip(tr("Заменить псевдопустые значения (0) на пустые"));
    connect(replace_empty_btn_, &QPushButton::clicked, this, &ManualInputDialog::onReplacePseudoEmpty);
    toolbar->addWidget(replace_empty_btn_);

    toolbar->addSeparator();

    auto* units_label = new QLabel(tr("Единицы углов:"), toolbar);
    toolbar->addWidget(units_label);

    angle_units_combo_ = new QComboBox(toolbar);
    angle_units_combo_->addItem(tr("Градусы"), static_cast<int>(AngleDisplayMode::DecimalDegrees));
    angle_units_combo_->addItem(tr("Градусы.минуты"), static_cast<int>(AngleDisplayMode::DegreesMinutes));
    connect(angle_units_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ManualInputDialog::onToggleAngleUnits);
    toolbar->addWidget(angle_units_combo_);

    layout->addWidget(toolbar);

    // Таблица измерений
    measurements_table_ = new QTableView(array_tab_);
    measurements_table_->setModel(measurements_model_.get());
    measurements_table_->setSelectionBehavior(QAbstractItemView::SelectRows);
    measurements_table_->setSelectionMode(QAbstractItemView::ExtendedSelection);
    measurements_table_->setAlternatingRowColors(true);
    measurements_table_->horizontalHeader()->setStretchLastSection(true);
    measurements_table_->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
    measurements_table_->verticalHeader()->setDefaultSectionSize(24);

    // Настройка ширины колонок
    measurements_table_->setColumnWidth(0, 100);  // Глубина
    measurements_table_->setColumnWidth(1, 100);  // Угол
    measurements_table_->setColumnWidth(2, 120);  // Азимут магн.
    measurements_table_->setColumnWidth(3, 120);  // Азимут ист.

    connect(measurements_model_.get(), &QAbstractItemModel::dataChanged,
            this, &ManualInputDialog::onCellChanged);

    layout->addWidget(measurements_table_, 1);

    // Подсказка
    auto* hint_label = new QLabel(
        tr("Совет: Значение азимута 0° или 360° означает направление на Север.\n"
           "Для отсутствия азимута используйте пустые значения, а не нули."),
        array_tab_);
    hint_label->setStyleSheet("color: #666; font-style: italic;");
    layout->addWidget(hint_label);

    tab_widget_->addTab(array_tab_, tr("Массив"));
}

void ManualInputDialog::loadFromWell() {
    if (!well_) return;

    const auto& meta = well_->metadata;

    // Идентификация
    uwi_edit_->setText(QString::fromStdString(meta.uwi));
    region_edit_->setText(QString::fromStdString(meta.region));
    field_edit_->setText(QString::fromStdString(meta.field_name));
    area_edit_->setText(QString::fromStdString(meta.area));
    pad_edit_->setText(QString::fromStdString(meta.well_pad));
    well_edit_->setText(QString::fromStdString(meta.well_name));
    measurement_number_edit_->setText(QString::fromStdString(meta.measurement_number));

    // Прибор
    device_edit_->setText(QString::fromStdString(meta.device));
    device_number_edit_->setText(QString::fromStdString(meta.device_number));
    if (!meta.device_calibration_date.empty()) {
        device_calibration_date_->setDate(
            QDate::fromString(QString::fromStdString(meta.device_calibration_date), "dd.MM.yyyy"));
    }

    // Интервал
    interval_start_spin_->setValue(meta.interval_start);
    interval_end_spin_->setValue(meta.interval_end);
    mag_declination_spin_->setValue(meta.magnetic_declination);
    kelly_bushing_spin_->setValue(meta.kelly_bushing);
    casing_shoe_spin_->setValue(meta.casing_shoe);

    // Альтитуда земли (для расчёта TVDBGL)
    // Не отображается в текущей версии диалога, но сохраняется в метаданных

    // Параметры скважины
    d_casing_spin_->setValue(meta.d_casing);
    d_collar_spin_->setValue(meta.d_collar);
    current_depth_spin_->setValue(meta.current_depth);
    project_depth_spin_->setValue(meta.project_depth);

    // Проектные параметры
    project_shift_spin_->setValue(meta.project_shift);
    project_azimuth_spin_->setValue(meta.project_azimuth);
    tolerance_radius_spin_->setValue(meta.tolerance_radius);

    // Погрешности
    angle_error_spin_->setValue(meta.angle_error);
    azimuth_error_spin_->setValue(meta.azimuth_error);

    // Организационные
    if (!meta.research_date.empty()) {
        research_date_->setDate(
            QDate::fromString(QString::fromStdString(meta.research_date), "dd.MM.yyyy"));
    }
    conditions_edit_->setText(QString::fromStdString(meta.conditions));
    research_type_edit_->setText(QString::fromStdString(meta.research_type));
    customer_edit_->setText(QString::fromStdString(meta.customer));
    contractor_edit_->setText(QString::fromStdString(meta.contractor));
    interpreter_edit_->setText(QString::fromStdString(meta.interpreter));
    party_chief_edit_->setText(QString::fromStdString(meta.party_chief));

    // Качество
    if (meta.quality == "good") {
        quality_combo_->setCurrentIndex(0);
    } else if (meta.quality == "satisfactory") {
        quality_combo_->setCurrentIndex(1);
    } else if (meta.quality == "poor") {
        quality_combo_->setCurrentIndex(2);
    }

    // Обновление счётчика точек
    points_count_label_->setText(tr("Точек: %1").arg(measurements_model_->rowCount()));
}

void ManualInputDialog::saveToWell() {
    if (!well_) return;

    auto& meta = well_->metadata;

    // Идентификация
    meta.uwi = uwi_edit_->text().toStdString();
    meta.region = region_edit_->text().toStdString();
    meta.field_name = field_edit_->text().toStdString();
    meta.area = area_edit_->text().toStdString();
    meta.well_pad = pad_edit_->text().toStdString();
    meta.well_name = well_edit_->text().toStdString();
    meta.measurement_number = measurement_number_edit_->text().toStdString();

    // Прибор
    meta.device = device_edit_->text().toStdString();
    meta.device_number = device_number_edit_->text().toStdString();
    meta.device_calibration_date = device_calibration_date_->date().toString("dd.MM.yyyy").toStdString();

    // Интервал
    meta.interval_start = interval_start_spin_->value();
    meta.interval_end = interval_end_spin_->value();
    meta.magnetic_declination = mag_declination_spin_->value();
    meta.kelly_bushing = kelly_bushing_spin_->value();
    meta.casing_shoe = casing_shoe_spin_->value();

    // Параметры скважины
    meta.d_casing = d_casing_spin_->value();
    meta.d_collar = d_collar_spin_->value();
    meta.current_depth = current_depth_spin_->value();
    meta.project_depth = project_depth_spin_->value();

    // Проектные параметры
    meta.project_shift = project_shift_spin_->value();
    meta.project_azimuth = project_azimuth_spin_->value();
    meta.tolerance_radius = tolerance_radius_spin_->value();

    // Погрешности
    meta.angle_error = angle_error_spin_->value();
    meta.azimuth_error = azimuth_error_spin_->value();

    // Также обновляем параметры расчёта
    well_->params.error_inclination_deg = meta.angle_error;
    well_->params.error_azimuth_deg = meta.azimuth_error;
    well_->params.magnetic_declination_deg = meta.magnetic_declination;
    well_->params.kelly_bushing_elevation_m = meta.kelly_bushing;
    well_->params.ground_elevation_m = meta.ground_elevation;

    // Организационные
    meta.research_date = research_date_->date().toString("dd.MM.yyyy").toStdString();
    meta.conditions = conditions_edit_->text().toStdString();
    meta.research_type = research_type_edit_->text().toStdString();
    meta.customer = customer_edit_->text().toStdString();
    meta.contractor = contractor_edit_->text().toStdString();
    meta.interpreter = interpreter_edit_->text().toStdString();
    meta.party_chief = party_chief_edit_->text().toStdString();

    // Качество
    meta.quality = quality_combo_->currentData().toString().toStdString();

    well_->modified = true;
}

std::shared_ptr<models::WellData> ManualInputDialog::wellData() const {
    return well_;
}

void ManualInputDialog::setWellData(std::shared_ptr<models::WellData> well) {
    well_ = well;
    if (!well_) {
        well_ = std::make_shared<models::WellData>();
    }
    measurements_model_->setWell(well_);
    loadFromWell();
}

void ManualInputDialog::onSave() {
    if (!validateInput()) {
        QMessageBox::warning(this, tr("Ошибка валидации"),
                             tr("Исправьте ошибки в данных перед сохранением."));
        return;
    }

    saveToWell();
    accept();
}

void ManualInputDialog::onCancel() {
    reject();
}

void ManualInputDialog::onAddRow() {
    int row = measurements_model_->rowCount();

    // Определение глубины для новой строки
    double new_depth = 0.0;
    if (row > 0) {
        auto last_point = well_->measurements.back();
        new_depth = last_point.measured_depth_m + 10.0;  // +10м по умолчанию
    }

    models::MeasuredPoint point;
    point.measured_depth_m = new_depth;
    point.inclination_deg = 0.0;

    measurements_model_->insertRow(row);
    measurements_model_->setData(measurements_model_->index(row, 0), new_depth);
    measurements_model_->setData(measurements_model_->index(row, 1), 0.0);

    // Выбор новой строки
    measurements_table_->selectRow(row);
    measurements_table_->scrollTo(measurements_model_->index(row, 0));
}

void ManualInputDialog::onRemoveRow() {
    auto selection = measurements_table_->selectionModel()->selectedRows();
    if (selection.isEmpty()) {
        return;
    }

    // Удаление с конца, чтобы индексы не сбивались
    QList<int> rows;
    for (const auto& idx : selection) {
        rows.append(idx.row());
    }
    std::sort(rows.begin(), rows.end(), std::greater<int>());

    for (int row : rows) {
        measurements_model_->removeRow(row);
    }
}

void ManualInputDialog::onImportFromClipboard() {
    QClipboard* clipboard = QApplication::clipboard();
    QString text = clipboard->text();

    if (text.isEmpty()) {
        QMessageBox::information(this, tr("Импорт"),
                                 tr("Буфер обмена пуст"));
        return;
    }

    // Разбор текста из буфера
    QStringList lines = text.split(QRegularExpression("[\r\n]+"), Qt::SkipEmptyParts);

    int imported = 0;
    for (const QString& line : lines) {
        // Разделители: табуляция, точка с запятой, запятая, пробелы
        QStringList parts = line.split(QRegularExpression("[\\t;,\\s]+"), Qt::SkipEmptyParts);

        if (parts.size() >= 2) {
            bool ok_depth, ok_angle;
            double depth = parts[0].replace(',', '.').toDouble(&ok_depth);
            double angle = parts[1].replace(',', '.').toDouble(&ok_angle);

            if (ok_depth && ok_angle) {
                models::MeasuredPoint point;
                point.measured_depth_m = depth;
                point.inclination_deg = angleToDecimal(angle);

                // Азимут магнитный (если есть)
                if (parts.size() >= 3) {
                    bool ok_azim;
                    double azim = parts[2].replace(',', '.').toDouble(&ok_azim);
                    if (ok_azim && !parts[2].isEmpty()) {
                        point.azimuth_deg = angleToDecimal(azim);
                        point.azimuth_type = models::AzimuthType::kMagnetic;
                    }
                }

                // Азимут истинный (если есть)
                if (parts.size() >= 4) {
                    bool ok_azim_true;
                    double azim_true = parts[3].replace(',', '.').toDouble(&ok_azim_true);
                    if (ok_azim_true && !parts[3].isEmpty()) {
                        point.azimuth_true_deg = angleToDecimal(azim_true);
                        // Если магнитный не задан, используем истинный как основной
                        if (!point.azimuth_deg.has_value()) {
                            point.azimuth_deg = point.azimuth_true_deg;
                            point.azimuth_type = models::AzimuthType::kTrue;
                        }
                    }
                }

                well_->measurements.push_back(point);
                ++imported;
            }
        }
    }

    if (imported > 0) {
        measurements_model_->refresh();
        QMessageBox::information(this, tr("Импорт"),
                                 tr("Импортировано точек: %1").arg(imported));
    } else {
        QMessageBox::warning(this, tr("Импорт"),
                             tr("Не удалось распознать данные в буфере обмена.\n"
                                "Ожидаемый формат: Глубина Угол [Азимут] [Азимут_ист.]"));
    }

    onValidateData();
}

void ManualInputDialog::onFlipArray() {
    if (well_->measurements.size() < 2) {
        return;
    }

    std::reverse(well_->measurements.begin(), well_->measurements.end());
    measurements_model_->refresh();

    QMessageBox::information(this, tr("Переворот"),
                             tr("Массив перевёрнут"));
}

void ManualInputDialog::onFlipColumn() {
    auto selection = measurements_table_->selectionModel()->selectedColumns();
    if (selection.isEmpty()) {
        QMessageBox::information(this, tr("Переворот"),
                                 tr("Выберите колонку для переворота"));
        return;
    }

    int col = selection.first().column();

    // Извлекаем значения
    std::vector<QVariant> values;
    for (size_t i = 0; i < well_->measurements.size(); ++i) {
        values.push_back(measurements_model_->data(measurements_model_->index(i, col)));
    }

    // Переворачиваем
    std::reverse(values.begin(), values.end());

    // Записываем обратно
    for (size_t i = 0; i < well_->measurements.size(); ++i) {
        measurements_model_->setData(measurements_model_->index(i, col), values[i]);
    }

    QMessageBox::information(this, tr("Переворот"),
                             tr("Колонка перевёрнута"));
}

void ManualInputDialog::onReplacePseudoEmpty() {
    int replaced = 0;

    for (auto& point : well_->measurements) {
        // Замена нулевых азимутов на пустые (только если это действительно "нет данных")
        // Внимание: 0° и 360° — это Север, не "пусто"!
        // Здесь пользователь должен явно выбрать, какие нули заменить
    }

    QMessageBox::information(this, tr("Замена"),
                             tr("Замена псевдопустых значений требует явного выбора.\n"
                                "Внимание: азимут 0° означает направление на Север!\n"
                                "Используйте редактирование ячеек для очистки значений."));
}

void ManualInputDialog::onToggleAngleUnits() {
    int idx = angle_units_combo_->currentIndex();
    AngleDisplayMode new_mode = static_cast<AngleDisplayMode>(
        angle_units_combo_->itemData(idx).toInt());

    if (new_mode == angle_mode_) {
        return;
    }

    // Конвертация значений в модели
    // На данный момент модель хранит градусы, отображение через делегат
    angle_mode_ = new_mode;

    // Обновление заголовков таблицы
    if (angle_mode_ == AngleDisplayMode::DegreesMinutes) {
        measurements_table_->model()->setHeaderData(1, Qt::Horizontal, tr("Угол (гр.мин)"));
        measurements_table_->model()->setHeaderData(2, Qt::Horizontal, tr("Азимут (гр.мин)"));
        measurements_table_->model()->setHeaderData(3, Qt::Horizontal, tr("Азимут ист. (гр.мин)"));
    } else {
        measurements_table_->model()->setHeaderData(1, Qt::Horizontal, tr("Угол (°)"));
        measurements_table_->model()->setHeaderData(2, Qt::Horizontal, tr("Азимут (°)"));
        measurements_table_->model()->setHeaderData(3, Qt::Horizontal, tr("Азимут ист. (°)"));
    }

    measurements_table_->viewport()->update();
}

void ManualInputDialog::onValidateData() {
    validateInput();
    updateErrorMessages();
    highlightErrors();
}

void ManualInputDialog::onCellChanged(const QModelIndex& /*index*/) {
    // Отложенная валидация после изменения
    QTimer::singleShot(100, this, &ManualInputDialog::onValidateData);
}

bool ManualInputDialog::validateInput() {
    QStringList errors;

    // Проверка метаданных
    if (well_edit_->text().isEmpty()) {
        errors << tr("Не указано название скважины");
    }

    // Проверка массива
    double prev_depth = -1.0;
    for (size_t i = 0; i < well_->measurements.size(); ++i) {
        const auto& pt = well_->measurements[i];

        // Глубина
        if (std::isnan(pt.measured_depth_m)) {
            errors << tr("Строка %1: отсутствует глубина").arg(i + 1);
        } else if (pt.measured_depth_m < 0) {
            errors << tr("Строка %1: отрицательная глубина").arg(i + 1);
        } else if (pt.measured_depth_m <= prev_depth) {
            errors << tr("Строка %1: нарушена монотонность глубин").arg(i + 1);
        }
        prev_depth = pt.measured_depth_m;

        // Угол
        if (std::isnan(pt.inclination_deg)) {
            errors << tr("Строка %1: отсутствует угол").arg(i + 1);
        } else if (pt.inclination_deg < 0 || pt.inclination_deg > max_angle_) {
            errors << tr("Строка %1: угол вне диапазона [0; %2]").arg(i + 1).arg(max_angle_);
        }

        // Азимут
        if (pt.azimuth_deg.has_value()) {
            double az = pt.azimuth_deg.value();
            if (az < 0 || az > 360) {
                errors << tr("Строка %1: азимут вне диапазона [0; 360]").arg(i + 1);
            }
        }
    }

    error_label_->setText(errors.isEmpty() ? "" : errors.join("; "));
    return errors.isEmpty();
}

void ManualInputDialog::updateErrorMessages() {
    // Уже реализовано в validateInput()
}

void ManualInputDialog::highlightErrors() {
    // TODO: подсветка ячеек с ошибками через делегат
}

double ManualInputDialog::angleToDecimal(double value) const {
    if (angle_mode_ == AngleDisplayMode::DegreesMinutes) {
        return utils::deg_from_degmin_value(value);
    }
    return value;
}

double ManualInputDialog::angleFromDecimal(double value) const {
    if (angle_mode_ == AngleDisplayMode::DegreesMinutes) {
        return utils::deg_to_degmin_value(value);
    }
    return value;
}

}  // namespace incline3d::ui

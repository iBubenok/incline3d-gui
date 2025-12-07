#include "ui/process_dialog.h"
#include "core/incline_process_runner.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QDir>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QMessageBox>
#include <QProgressBar>
#include <QPushButton>
#include <QTabWidget>
#include <QTemporaryFile>
#include <QTextEdit>
#include <QTimer>

#include "utils/logger.h"

namespace incline3d::ui {

ProcessDialog::ProcessDialog(std::shared_ptr<models::WellData> well,
                             core::InclineProcessRunner* runner,
                             QWidget* parent)
    : QDialog(parent)
    , well_(well)
    , runner_(runner) {
    setWindowTitle(tr("Обработка скважины - %1")
        .arg(QString::fromStdString(well_->metadata.well_name)));
    setMinimumSize(600, 550);
    setupUi();
    loadParams();
}

void ProcessDialog::setupUi() {
    auto* main_layout = new QVBoxLayout(this);

    tab_widget_ = new QTabWidget(this);
    createMethodTab();
    createAzimuthTab();
    createElevationTab();
    createQualityTab();
    main_layout->addWidget(tab_widget_);

    // Лог
    auto* log_group = new QGroupBox(tr("Журнал обработки"), this);
    auto* log_layout = new QVBoxLayout(log_group);
    log_text_ = new QTextEdit(log_group);
    log_text_->setReadOnly(true);
    log_text_->setMaximumHeight(100);
    log_layout->addWidget(log_text_);

    progress_bar_ = new QProgressBar(log_group);
    progress_bar_->setVisible(false);
    log_layout->addWidget(progress_bar_);

    main_layout->addWidget(log_group);

    // Кнопки
    auto* button_layout = new QHBoxLayout();
    button_layout->addStretch();

    process_btn_ = new QPushButton(tr("Обработать"), this);
    connect(process_btn_, &QPushButton::clicked, this, &ProcessDialog::onProcess);
    button_layout->addWidget(process_btn_);

    auto* close_btn = new QPushButton(tr("Закрыть"), this);
    connect(close_btn, &QPushButton::clicked, this, &QDialog::accept);
    button_layout->addWidget(close_btn);

    main_layout->addLayout(button_layout);
}

void ProcessDialog::createMethodTab() {
    auto* tab = new QWidget();
    auto* layout = new QVBoxLayout(tab);

    // Группа метода расчёта
    auto* method_group = new QGroupBox(tr("Метод расчёта траектории"), tab);
    auto* method_layout = new QFormLayout(method_group);

    method_combo_ = new QComboBox(method_group);
    method_combo_->addItem(tr("Минимальная кривизна (по умолчанию)"), "mincurv");
    method_combo_->addItem(tr("Сбалансированная тангента"), "balanced");
    method_combo_->addItem(tr("Средний угол"), "average");
    method_combo_->addItem(tr("Радиус кривизны"), "radiuscurv");
    method_combo_->addItem(tr("Кольцевая дуга"), "ringarc");
    method_layout->addRow(tr("Метод:"), method_combo_);

    layout->addWidget(method_group);

    // Группа интенсивности
    auto* intensity_group = new QGroupBox(tr("Расчёт интенсивности"), tab);
    auto* intensity_layout = new QFormLayout(intensity_group);

    intensity_interval_spin_ = new QDoubleSpinBox(intensity_group);
    intensity_interval_spin_->setRange(1, 100);
    intensity_interval_spin_->setValue(30);
    intensity_interval_spin_->setSuffix(tr(" м"));
    intensity_interval_spin_->setToolTip(tr("Интервал L для расчёта интенсивности (по умолчанию 30 м)"));
    intensity_layout->addRow(tr("Интервал L:"), intensity_interval_spin_);

    smooth_intensity_check_ = new QCheckBox(tr("Сглаживание интенсивности"), intensity_group);
    smooth_intensity_check_->setToolTip(tr("Сглаживание в скользящем окне"));
    intensity_layout->addRow("", smooth_intensity_check_);

    layout->addWidget(intensity_group);
    layout->addStretch();

    tab_widget_->addTab(tab, tr("Метод"));
}

void ProcessDialog::createAzimuthTab() {
    auto* tab = new QWidget();
    auto* layout = new QVBoxLayout(tab);

    // Группа типа азимута
    auto* type_group = new QGroupBox(tr("Тип азимута"), tab);
    auto* type_layout = new QFormLayout(type_group);

    azimuth_type_combo_ = new QComboBox(type_group);
    azimuth_type_combo_->addItem(tr("Магнитный (по умолчанию)"), "magnetic");
    azimuth_type_combo_->addItem(tr("Истинный (географический)"), "true");
    azimuth_type_combo_->addItem(tr("Дирекционный угол (сеточный)"), "grid");
    connect(azimuth_type_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ProcessDialog::onAzimuthModeChanged);
    type_layout->addRow(tr("Тип азимута в данных:"), azimuth_type_combo_);

    layout->addWidget(type_group);

    // Группа поправок
    auto* corrections_group = new QGroupBox(tr("Поправки азимута"), tab);
    auto* corrections_layout = new QFormLayout(corrections_group);

    declination_spin_ = new QDoubleSpinBox(corrections_group);
    declination_spin_->setRange(-180, 180);
    declination_spin_->setDecimals(2);
    declination_spin_->setSuffix(tr("°"));
    declination_spin_->setToolTip(tr("Магнитное склонение: положительное на восток, отрицательное на запад"));
    corrections_layout->addRow(tr("Магнитное склонение:"), declination_spin_);

    meridian_spin_ = new QDoubleSpinBox(corrections_group);
    meridian_spin_->setRange(-180, 180);
    meridian_spin_->setDecimals(2);
    meridian_spin_->setSuffix(tr("°"));
    meridian_spin_->setToolTip(tr("Сближение меридианов для перехода от дирекционного угла к истинному"));
    corrections_layout->addRow(tr("Сближение меридианов:"), meridian_spin_);

    use_summary_correction_check_ = new QCheckBox(
        tr("Использовать суммарную поправку (магн. склонение уже включает сближение)"),
        corrections_group);
    corrections_layout->addRow("", use_summary_correction_check_);

    layout->addWidget(corrections_group);

    // Группа обработки пропущенных азимутов
    auto* blank_group = new QGroupBox(tr("Обработка пропущенных азимутов"), tab);
    auto* blank_layout = new QVBoxLayout(blank_group);

    use_last_azimuth_check_ = new QCheckBox(
        tr("Использовать предыдущий азимут для пропущенных точек"), blank_group);
    use_last_azimuth_check_->setChecked(true);
    blank_layout->addWidget(use_last_azimuth_check_);

    interpolate_azimuths_check_ = new QCheckBox(
        tr("Интерполировать пропущенные азимуты между известными"), blank_group);
    interpolate_azimuths_check_->setChecked(true);
    blank_layout->addWidget(interpolate_azimuths_check_);

    unwrap_azimuths_check_ = new QCheckBox(
        tr("Разворачивать азимуты (устранять скачки через 0/360°)"), blank_group);
    unwrap_azimuths_check_->setChecked(true);
    blank_layout->addWidget(unwrap_azimuths_check_);

    continuous_mode_check_ = new QCheckBox(
        tr("Непрерывный инклинометр (данные по всему стволу)"), blank_group);
    blank_layout->addWidget(continuous_mode_check_);

    layout->addWidget(blank_group);

    // Группа SNGF-режима
    auto* sngf_group = new QGroupBox(tr("SNGF-режим (вертикальные участки)"), tab);
    auto* sngf_layout = new QFormLayout(sngf_group);

    sngf_mode_check_ = new QCheckBox(tr("Включить SNGF-режим"), sngf_group);
    sngf_mode_check_->setToolTip(tr("Специальная обработка вертикальных участков: азимуты не интерполируются"));
    connect(sngf_mode_check_, &QCheckBox::toggled, this, &ProcessDialog::onSngfModeChanged);
    sngf_layout->addRow("", sngf_mode_check_);

    sngf_min_angle_spin_ = new QDoubleSpinBox(sngf_group);
    sngf_min_angle_spin_->setRange(0, 30);
    sngf_min_angle_spin_->setDecimals(1);
    sngf_min_angle_spin_->setValue(5.0);
    sngf_min_angle_spin_->setSuffix(tr("°"));
    sngf_min_angle_spin_->setToolTip(tr("Минимальный угол для применения SNGF-режима"));
    sngf_min_angle_spin_->setEnabled(false);
    sngf_layout->addRow(tr("Мин. угол:"), sngf_min_angle_spin_);

    vertical_limit_spin_ = new QDoubleSpinBox(sngf_group);
    vertical_limit_spin_->setRange(0, 30);
    vertical_limit_spin_->setDecimals(1);
    vertical_limit_spin_->setValue(3.0);
    vertical_limit_spin_->setSuffix(tr("°"));
    vertical_limit_spin_->setToolTip(tr("Предел угла для вертикального участка (азимут не учитывается)"));
    sngf_layout->addRow(tr("Вертикальный предел:"), vertical_limit_spin_);

    layout->addWidget(sngf_group);
    layout->addStretch();

    tab_widget_->addTab(tab, tr("Азимуты"));
}

void ProcessDialog::createElevationTab() {
    auto* tab = new QWidget();
    auto* layout = new QVBoxLayout(tab);

    // Группа высотных отметок
    auto* elevation_group = new QGroupBox(tr("Высотные отметки"), tab);
    auto* elevation_layout = new QFormLayout(elevation_group);

    kelly_bushing_spin_ = new QDoubleSpinBox(elevation_group);
    kelly_bushing_spin_->setRange(-1000, 10000);
    kelly_bushing_spin_->setDecimals(2);
    kelly_bushing_spin_->setSuffix(tr(" м"));
    kelly_bushing_spin_->setToolTip(tr("Альтитуда стола ротора (устья скважины) над уровнем моря"));
    elevation_layout->addRow(tr("Альтитуда устья (KB):"), kelly_bushing_spin_);

    ground_elevation_spin_ = new QDoubleSpinBox(elevation_group);
    ground_elevation_spin_->setRange(-1000, 10000);
    ground_elevation_spin_->setDecimals(2);
    ground_elevation_spin_->setSuffix(tr(" м"));
    ground_elevation_spin_->setToolTip(tr("Альтитуда поверхности земли для расчёта TVDBGL"));
    elevation_layout->addRow(tr("Альтитуда земли:"), ground_elevation_spin_);

    water_depth_spin_ = new QDoubleSpinBox(elevation_group);
    water_depth_spin_->setRange(0, 5000);
    water_depth_spin_->setDecimals(2);
    water_depth_spin_->setSuffix(tr(" м"));
    water_depth_spin_->setToolTip(tr("Глубина воды для морских скважин (для расчёта TVDBML)"));
    elevation_layout->addRow(tr("Глубина воды:"), water_depth_spin_);

    layout->addWidget(elevation_group);

    // Группа расчёта глубин
    auto* tvd_group = new QGroupBox(tr("Расчёт глубин"), tab);
    auto* tvd_layout = new QVBoxLayout(tvd_group);

    calculate_tvd_bgl_check_ = new QCheckBox(
        tr("Рассчитать TVDBGL (TVD ниже уровня земли)"), tvd_group);
    tvd_layout->addWidget(calculate_tvd_bgl_check_);

    calculate_tvd_bml_check_ = new QCheckBox(
        tr("Рассчитать TVDBML (TVD ниже уровня моря)"), tvd_group);
    tvd_layout->addWidget(calculate_tvd_bml_check_);

    layout->addWidget(tvd_group);
    layout->addStretch();

    tab_widget_->addTab(tab, tr("Высоты"));
}

void ProcessDialog::createQualityTab() {
    auto* tab = new QWidget();
    auto* layout = new QVBoxLayout(tab);

    // Группа контроля качества
    auto* quality_group = new QGroupBox(tr("Контроль качества"), tab);
    auto* quality_layout = new QFormLayout(quality_group);

    quality_check_check_ = new QCheckBox(tr("Включить контроль качества"), quality_group);
    quality_layout->addRow("", quality_check_check_);

    max_angle_deviation_spin_ = new QDoubleSpinBox(quality_group);
    max_angle_deviation_spin_->setRange(0, 30);
    max_angle_deviation_spin_->setDecimals(1);
    max_angle_deviation_spin_->setValue(5.0);
    max_angle_deviation_spin_->setSuffix(tr("°"));
    max_angle_deviation_spin_->setToolTip(tr("Максимально допустимое отклонение угла между соседними точками"));
    quality_layout->addRow(tr("Макс. отклонение угла:"), max_angle_deviation_spin_);

    max_azimuth_deviation_spin_ = new QDoubleSpinBox(quality_group);
    max_azimuth_deviation_spin_->setRange(0, 90);
    max_azimuth_deviation_spin_->setDecimals(1);
    max_azimuth_deviation_spin_->setValue(10.0);
    max_azimuth_deviation_spin_->setSuffix(tr("°"));
    max_azimuth_deviation_spin_->setToolTip(tr("Максимально допустимое отклонение азимута между соседними точками"));
    quality_layout->addRow(tr("Макс. отклонение азимута:"), max_azimuth_deviation_spin_);

    intensity_threshold_spin_ = new QDoubleSpinBox(quality_group);
    intensity_threshold_spin_->setRange(0, 30);
    intensity_threshold_spin_->setDecimals(1);
    intensity_threshold_spin_->setValue(0);
    intensity_threshold_spin_->setSuffix(tr("°/10м"));
    intensity_threshold_spin_->setToolTip(tr("Порог интенсивности для предупреждения (0 = не проверять)"));
    quality_layout->addRow(tr("Порог интенсивности:"), intensity_threshold_spin_);

    delta_depth_warning_spin_ = new QDoubleSpinBox(quality_group);
    delta_depth_warning_spin_->setRange(0, 100);
    delta_depth_warning_spin_->setDecimals(1);
    delta_depth_warning_spin_->setValue(0);
    delta_depth_warning_spin_->setSuffix(tr(" м"));
    delta_depth_warning_spin_->setToolTip(tr("Минимальный шаг глубины для предупреждения (0 = не проверять)"));
    quality_layout->addRow(tr("Мин. шаг глубины:"), delta_depth_warning_spin_);

    layout->addWidget(quality_group);

    // Группа погрешностей
    auto* errors_group = new QGroupBox(tr("Погрешности измерений (для расчёта ошибок координат)"), tab);
    auto* errors_layout = new QFormLayout(errors_group);

    error_depth_spin_ = new QDoubleSpinBox(errors_group);
    error_depth_spin_->setRange(0, 10);
    error_depth_spin_->setDecimals(3);
    error_depth_spin_->setValue(0.1);
    error_depth_spin_->setSuffix(tr(" м"));
    errors_layout->addRow(tr("Погрешность глубины:"), error_depth_spin_);

    error_angle_spin_ = new QDoubleSpinBox(errors_group);
    error_angle_spin_->setRange(0, 5);
    error_angle_spin_->setDecimals(2);
    error_angle_spin_->setValue(0.1);
    error_angle_spin_->setSuffix(tr("°"));
    errors_layout->addRow(tr("Погрешность угла:"), error_angle_spin_);

    error_azimuth_spin_ = new QDoubleSpinBox(errors_group);
    error_azimuth_spin_->setRange(0, 5);
    error_azimuth_spin_->setDecimals(2);
    error_azimuth_spin_->setValue(0.1);
    error_azimuth_spin_->setSuffix(tr("°"));
    errors_layout->addRow(tr("Погрешность азимута:"), error_azimuth_spin_);

    layout->addWidget(errors_group);
    layout->addStretch();

    tab_widget_->addTab(tab, tr("Качество"));
}

void ProcessDialog::loadParams() {
    if (!well_) return;

    const auto& p = well_->params;
    const auto& meta = well_->metadata;

    // Метод
    QString method_str = QString::fromStdString(models::method_to_string(p.method));
    int idx = method_combo_->findData(method_str);
    if (idx >= 0) method_combo_->setCurrentIndex(idx);

    intensity_interval_spin_->setValue(p.intensity_interval_m);
    smooth_intensity_check_->setChecked(p.smooth_intensity);

    // Азимуты
    QString azim_type_str = QString::fromStdString(models::azimuth_type_to_string(p.azimuth_type));
    int azim_idx = azimuth_type_combo_->findData(azim_type_str);
    if (azim_idx >= 0) azimuth_type_combo_->setCurrentIndex(azim_idx);

    declination_spin_->setValue(p.magnetic_declination_deg);
    meridian_spin_->setValue(p.meridian_convergence_deg);

    use_last_azimuth_check_->setChecked(p.use_last_azimuth);
    interpolate_azimuths_check_->setChecked(p.interpolate_missing_azimuths);
    unwrap_azimuths_check_->setChecked(p.unwrap_azimuths);

    sngf_mode_check_->setChecked(p.sngf_mode);
    sngf_min_angle_spin_->setValue(p.sngf_min_angle_deg);
    sngf_min_angle_spin_->setEnabled(p.sngf_mode);

    vertical_limit_spin_->setValue(p.vertical_limit_deg);

    // Высоты
    kelly_bushing_spin_->setValue(meta.kelly_bushing);
    ground_elevation_spin_->setValue(meta.ground_elevation);
    water_depth_spin_->setValue(p.water_depth_m);

    // Качество
    quality_check_check_->setChecked(p.quality_check);
    max_angle_deviation_spin_->setValue(p.max_angle_deviation_deg);
    max_azimuth_deviation_spin_->setValue(p.max_azimuth_deviation_deg);
    intensity_threshold_spin_->setValue(p.intensity_threshold_deg);
    delta_depth_warning_spin_->setValue(p.delta_depth_warning_m);

    error_depth_spin_->setValue(p.error_depth_m);
    error_angle_spin_->setValue(p.error_inclination_deg);
    error_azimuth_spin_->setValue(p.error_azimuth_deg);
}

void ProcessDialog::saveParams() {
    if (!well_) return;

    auto& p = well_->params;
    auto& meta = well_->metadata;

    // Метод
    p.method = models::string_to_method(method_combo_->currentData().toString().toStdString());
    p.intensity_interval_m = intensity_interval_spin_->value();
    p.smooth_intensity = smooth_intensity_check_->isChecked();

    // Азимуты
    p.azimuth_type = models::string_to_azimuth_type(
        azimuth_type_combo_->currentData().toString().toStdString());
    p.magnetic_declination_deg = declination_spin_->value();
    p.meridian_convergence_deg = meridian_spin_->value();

    p.use_last_azimuth = use_last_azimuth_check_->isChecked();
    p.interpolate_missing_azimuths = interpolate_azimuths_check_->isChecked();
    p.unwrap_azimuths = unwrap_azimuths_check_->isChecked();

    p.sngf_mode = sngf_mode_check_->isChecked();
    p.sngf_min_angle_deg = sngf_min_angle_spin_->value();
    p.vertical_limit_deg = vertical_limit_spin_->value();

    // Высоты
    meta.kelly_bushing = kelly_bushing_spin_->value();
    meta.ground_elevation = ground_elevation_spin_->value();
    p.kelly_bushing_elevation_m = meta.kelly_bushing;
    p.ground_elevation_m = meta.ground_elevation;
    p.water_depth_m = water_depth_spin_->value();

    // Качество
    p.quality_check = quality_check_check_->isChecked();
    p.max_angle_deviation_deg = max_angle_deviation_spin_->value();
    p.max_azimuth_deviation_deg = max_azimuth_deviation_spin_->value();
    p.intensity_threshold_deg = intensity_threshold_spin_->value();
    p.delta_depth_warning_m = delta_depth_warning_spin_->value();

    p.error_depth_m = error_depth_spin_->value();
    p.error_inclination_deg = error_angle_spin_->value();
    p.error_azimuth_deg = error_azimuth_spin_->value();
}

void ProcessDialog::onAzimuthModeChanged(int index) {
    QString mode = azimuth_type_combo_->itemData(index).toString();

    // Показывать/скрывать поля поправок в зависимости от типа азимута
    bool is_magnetic = (mode == "magnetic");
    declination_spin_->setEnabled(is_magnetic);

    bool is_grid = (mode == "grid");
    meridian_spin_->setEnabled(is_magnetic || is_grid);
}

void ProcessDialog::onSngfModeChanged(bool enabled) {
    sngf_min_angle_spin_->setEnabled(enabled);
}

void ProcessDialog::onProcess() {
    saveParams();
    log_text_->clear();
    log_text_->append(tr("Начало обработки скважины: %1")
        .arg(QString::fromStdString(well_->metadata.well_name)));
    log_text_->append(tr("Метод: %1").arg(method_combo_->currentText()));

    process_btn_->setEnabled(false);
    progress_bar_->setVisible(true);
    progress_bar_->setRange(0, 0);  // Индикатор "бесконечная" загрузка

    // Здесь должен быть вызов inclproc через InclineProcessRunner
    // Пока показываем сообщение о необходимости интеграции

    // Симуляция задержки обработки
    QTimer::singleShot(500, this, &ProcessDialog::onProcessFinished);
}

void ProcessDialog::onProcessFinished() {
    process_btn_->setEnabled(true);
    progress_bar_->setVisible(false);

    log_text_->append(tr("Обработка завершена"));
    log_text_->append(tr("Точек обработано: %1").arg(well_->measurements.size()));

    QMessageBox::information(this, tr("Обработка"),
        tr("Параметры сохранены.\n\n"
           "Для выполнения расчёта требуется интеграция с CLI inclproc.\n"
           "После интеграции результаты будут записаны в скважину."));

    well_->modified = true;
}

}  // namespace incline3d::ui

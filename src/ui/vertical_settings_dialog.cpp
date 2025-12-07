#include "ui/vertical_settings_dialog.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QTabWidget>

namespace incline3d::ui {

VerticalSettingsDialog::VerticalSettingsDialog(QWidget* parent)
    : QDialog(parent) {
    setupUi();
}

VerticalSettingsDialog::~VerticalSettingsDialog() = default;

void VerticalSettingsDialog::setupUi() {
    setWindowTitle(tr("Настройки вертикальной проекции"));
    setMinimumSize(500, 500);

    auto* main_layout = new QVBoxLayout(this);

    auto* tab_widget = new QTabWidget(this);
    createProjectionTab();
    createHeaderTab();

    auto* projection_tab = new QWidget();
    auto* projection_layout = new QVBoxLayout(projection_tab);

    // Группа азимута
    auto* azimuth_group = new QGroupBox(tr("Азимут профиля"), projection_tab);
    auto* azimuth_layout = new QFormLayout(azimuth_group);

    auto* azimuth_row = new QHBoxLayout();
    azimuth_spin_ = new QDoubleSpinBox(azimuth_group);
    azimuth_spin_->setRange(0, 360);
    azimuth_spin_->setDecimals(1);
    azimuth_spin_->setSuffix(tr("°"));
    azimuth_row->addWidget(azimuth_spin_);

    auto_fit_check_ = new QCheckBox(tr("Автоподбор"), azimuth_group);
    auto_fit_check_->setChecked(true);
    connect(auto_fit_check_, &QCheckBox::toggled, this, &VerticalSettingsDialog::onAutoFitChanged);
    azimuth_row->addWidget(auto_fit_check_);

    fit_now_btn_ = new QPushButton(tr("Подобрать"), azimuth_group);
    fit_now_btn_->setToolTip(tr("Подобрать оптимальный азимут для максимального смещения"));
    azimuth_row->addWidget(fit_now_btn_);
    azimuth_row->addStretch();

    azimuth_layout->addRow(tr("Азимут:"), azimuth_row);
    projection_layout->addWidget(azimuth_group);

    // Группа масштаба
    auto* scale_group = new QGroupBox(tr("Масштаб"), projection_tab);
    auto* scale_layout = new QFormLayout(scale_group);

    h_scale_spin_ = new QDoubleSpinBox(scale_group);
    h_scale_spin_->setRange(1, 10000);
    h_scale_spin_->setDecimals(1);
    h_scale_spin_->setSuffix(tr(" м/см"));
    h_scale_spin_->setValue(100);
    connect(h_scale_spin_, QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this, &VerticalSettingsDialog::onHorizontalScaleChanged);
    scale_layout->addRow(tr("Горизонтальный:"), h_scale_spin_);

    v_scale_spin_ = new QDoubleSpinBox(scale_group);
    v_scale_spin_->setRange(1, 10000);
    v_scale_spin_->setDecimals(1);
    v_scale_spin_->setSuffix(tr(" м/см"));
    v_scale_spin_->setValue(100);
    scale_layout->addRow(tr("Вертикальный:"), v_scale_spin_);

    link_scales_check_ = new QCheckBox(tr("Связать масштабы"), scale_group);
    link_scales_check_->setChecked(true);
    connect(link_scales_check_, &QCheckBox::toggled, this, &VerticalSettingsDialog::onLinkScalesChanged);
    scale_layout->addRow("", link_scales_check_);

    projection_layout->addWidget(scale_group);

    // Группа сетки
    auto* grid_group = new QGroupBox(tr("Сетка и подписи"), projection_tab);
    auto* grid_layout = new QFormLayout(grid_group);

    show_grid_check_ = new QCheckBox(tr("Показывать сетку"), grid_group);
    show_grid_check_->setChecked(true);
    grid_layout->addRow("", show_grid_check_);

    grid_step_spin_ = new QDoubleSpinBox(grid_group);
    grid_step_spin_->setRange(1, 1000);
    grid_step_spin_->setDecimals(1);
    grid_step_spin_->setSuffix(tr(" м"));
    grid_step_spin_->setValue(50);
    grid_layout->addRow(tr("Шаг сетки:"), grid_step_spin_);

    show_depth_labels_check_ = new QCheckBox(tr("Показывать глубины"), grid_group);
    show_depth_labels_check_->setChecked(true);
    grid_layout->addRow("", show_depth_labels_check_);

    depth_label_step_spin_ = new QSpinBox(grid_group);
    depth_label_step_spin_->setRange(10, 1000);
    depth_label_step_spin_->setSuffix(tr(" м"));
    depth_label_step_spin_->setValue(100);
    grid_layout->addRow(tr("Шаг подписей:"), depth_label_step_spin_);

    projection_layout->addWidget(grid_group);

    // Группа дополнительных элементов
    auto* extras_group = new QGroupBox(tr("Дополнительно"), projection_tab);
    auto* extras_layout = new QFormLayout(extras_group);

    show_project_points_check_ = new QCheckBox(tr("Показывать проектные точки"), extras_group);
    show_project_points_check_->setChecked(true);
    extras_layout->addRow("", show_project_points_check_);

    show_sea_level_check_ = new QCheckBox(tr("Показывать уровень моря"), extras_group);
    show_sea_level_check_->setChecked(true);
    extras_layout->addRow("", show_sea_level_check_);

    kelly_bushing_spin_ = new QDoubleSpinBox(extras_group);
    kelly_bushing_spin_->setRange(-1000, 10000);
    kelly_bushing_spin_->setDecimals(2);
    kelly_bushing_spin_->setSuffix(tr(" м"));
    kelly_bushing_spin_->setToolTip(tr("Альтитуда стола ротора для расчёта уровня моря"));
    extras_layout->addRow(tr("Альтитуда:"), kelly_bushing_spin_);

    projection_layout->addWidget(extras_group);
    projection_layout->addStretch();

    tab_widget->addTab(projection_tab, tr("Проекция"));

    // Вкладка шапки
    auto* header_tab = new QWidget();
    auto* header_layout = new QVBoxLayout(header_tab);

    auto* header_group = new QGroupBox(tr("Шапка вертикальной проекции"), header_tab);
    auto* header_form = new QFormLayout(header_group);

    header_title_edit_ = new QLineEdit(header_group);
    header_title_edit_->setPlaceholderText(tr("Вертикальная проекция траектории"));
    header_form->addRow(tr("Заголовок:"), header_title_edit_);

    header_well_edit_ = new QLineEdit(header_group);
    header_form->addRow(tr("Скважина:"), header_well_edit_);

    header_field_edit_ = new QLineEdit(header_group);
    header_form->addRow(tr("Месторождение:"), header_field_edit_);

    header_pad_edit_ = new QLineEdit(header_group);
    header_form->addRow(tr("Куст:"), header_pad_edit_);

    header_date_edit_ = new QLineEdit(header_group);
    header_form->addRow(tr("Дата:"), header_date_edit_);

    header_scale_edit_ = new QLineEdit(header_group);
    header_scale_edit_->setPlaceholderText(tr("1:1000"));
    header_form->addRow(tr("Масштаб (текст):"), header_scale_edit_);

    header_layout->addWidget(header_group);
    header_layout->addStretch();

    tab_widget->addTab(header_tab, tr("Шапка"));

    main_layout->addWidget(tab_widget);

    // Кнопки
    auto* button_layout = new QHBoxLayout();

    auto* apply_btn = new QPushButton(tr("Применить"), this);
    connect(apply_btn, &QPushButton::clicked, this, &VerticalSettingsDialog::onApply);
    button_layout->addWidget(apply_btn);

    button_layout->addStretch();

    auto* button_box = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(button_box, &QDialogButtonBox::accepted, this, [this]() {
        emitSettings();
        accept();
    });
    connect(button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    button_layout->addWidget(button_box);

    main_layout->addLayout(button_layout);
}

void VerticalSettingsDialog::createProjectionTab() {
    // Реализовано в setupUi
}

void VerticalSettingsDialog::createHeaderTab() {
    // Реализовано в setupUi
}

VerticalProjectionSettings VerticalSettingsDialog::settings() const {
    VerticalProjectionSettings s;
    s.azimuth_deg = azimuth_spin_->value();
    s.auto_fit_azimuth = auto_fit_check_->isChecked();
    s.horizontal_scale = h_scale_spin_->value();
    s.vertical_scale = v_scale_spin_->value();
    s.link_scales = link_scales_check_->isChecked();
    s.grid_step = grid_step_spin_->value();
    s.show_grid = show_grid_check_->isChecked();
    s.show_depth_labels = show_depth_labels_check_->isChecked();
    s.depth_label_step = depth_label_step_spin_->value();
    s.show_project_points = show_project_points_check_->isChecked();
    s.show_sea_level = show_sea_level_check_->isChecked();
    s.kelly_bushing = kelly_bushing_spin_->value();
    s.header_title = header_title_edit_->text();
    s.header_well = header_well_edit_->text();
    s.header_field = header_field_edit_->text();
    s.header_pad = header_pad_edit_->text();
    s.header_date = header_date_edit_->text();
    s.header_scale = header_scale_edit_->text();
    return s;
}

void VerticalSettingsDialog::setSettings(const VerticalProjectionSettings& s) {
    azimuth_spin_->setValue(s.azimuth_deg);
    auto_fit_check_->setChecked(s.auto_fit_azimuth);
    h_scale_spin_->setValue(s.horizontal_scale);
    v_scale_spin_->setValue(s.vertical_scale);
    link_scales_check_->setChecked(s.link_scales);
    grid_step_spin_->setValue(s.grid_step);
    show_grid_check_->setChecked(s.show_grid);
    show_depth_labels_check_->setChecked(s.show_depth_labels);
    depth_label_step_spin_->setValue(s.depth_label_step);
    show_project_points_check_->setChecked(s.show_project_points);
    show_sea_level_check_->setChecked(s.show_sea_level);
    kelly_bushing_spin_->setValue(s.kelly_bushing);
    header_title_edit_->setText(s.header_title);
    header_well_edit_->setText(s.header_well);
    header_field_edit_->setText(s.header_field);
    header_pad_edit_->setText(s.header_pad);
    header_date_edit_->setText(s.header_date);
    header_scale_edit_->setText(s.header_scale);

    onAutoFitChanged(s.auto_fit_azimuth);
    onLinkScalesChanged(s.link_scales);
}

void VerticalSettingsDialog::onAutoFitChanged(bool enabled) {
    azimuth_spin_->setEnabled(!enabled);
    fit_now_btn_->setEnabled(!enabled);
}

void VerticalSettingsDialog::onLinkScalesChanged(bool linked) {
    if (linked) {
        v_scale_spin_->setValue(h_scale_spin_->value());
        v_scale_spin_->setEnabled(false);
    } else {
        v_scale_spin_->setEnabled(true);
    }
}

void VerticalSettingsDialog::onHorizontalScaleChanged(double value) {
    if (link_scales_check_->isChecked()) {
        v_scale_spin_->setValue(value);
    }
}

void VerticalSettingsDialog::onApply() {
    emitSettings();
}

void VerticalSettingsDialog::emitSettings() {
    emit settingsChanged(settings());
}

}  // namespace incline3d::ui

#include "ui/view_options_dialog.h"

#include <QCheckBox>
#include <QColorDialog>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QTabWidget>
#include <QVBoxLayout>

namespace incline3d::ui {

ViewOptionsDialog::ViewOptionsDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Настройки отображения"));
    setMinimumSize(500, 450);
    setupUi();
}

void ViewOptionsDialog::setupUi() {
    auto* main_layout = new QVBoxLayout(this);

    tab_widget_ = new QTabWidget(this);

    // Создание вкладок
    auto* general_tab = new QWidget();
    setupGeneralTab(general_tab);
    tab_widget_->addTab(general_tab, tr("Общие"));

    auto* trajectory_tab = new QWidget();
    setupTrajectoryTab(trajectory_tab);
    tab_widget_->addTab(trajectory_tab, tr("Траектории"));

    auto* points_tab = new QWidget();
    setupPointsTab(points_tab);
    tab_widget_->addTab(points_tab, tr("Точки"));

    auto* tab_3d = new QWidget();
    setup3DTab(tab_3d);
    tab_widget_->addTab(tab_3d, tr("3D вид"));

    auto* colors_tab = new QWidget();
    setupColorsTab(colors_tab);
    tab_widget_->addTab(colors_tab, tr("Цвета"));

    main_layout->addWidget(tab_widget_);

    // Кнопки
    auto* button_layout = new QHBoxLayout();

    auto* apply_button = new QPushButton(tr("Применить"), this);
    connect(apply_button, &QPushButton::clicked, this, &ViewOptionsDialog::onApply);
    button_layout->addWidget(apply_button);

    auto* button_box = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(button_box, &QDialogButtonBox::accepted, this, [this]() {
        onApply();
        accept();
    });
    connect(button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    button_layout->addWidget(button_box);

    main_layout->addLayout(button_layout);
}

void ViewOptionsDialog::setupGeneralTab(QWidget* tab) {
    auto* layout = new QVBoxLayout(tab);

    // Группа элементов
    auto* elements_group = new QGroupBox(tr("Отображение элементов"), tab);
    auto* elements_layout = new QVBoxLayout(elements_group);

    show_grid_check_ = new QCheckBox(tr("Показывать сетку"), tab);
    elements_layout->addWidget(show_grid_check_);

    show_axes_check_ = new QCheckBox(tr("Показывать оси координат"), tab);
    elements_layout->addWidget(show_axes_check_);

    show_labels_check_ = new QCheckBox(tr("Показывать подписи"), tab);
    elements_layout->addWidget(show_labels_check_);

    show_depth_marks_check_ = new QCheckBox(tr("Показывать отметки глубины"), tab);
    elements_layout->addWidget(show_depth_marks_check_);

    layout->addWidget(elements_group);

    // Группа размеров
    auto* size_group = new QGroupBox(tr("Размеры"), tab);
    auto* size_layout = new QFormLayout(size_group);

    grid_step_spin_ = new QDoubleSpinBox(tab);
    grid_step_spin_->setRange(1, 10000);
    grid_step_spin_->setValue(100);
    grid_step_spin_->setSuffix(tr(" м"));
    size_layout->addRow(tr("Шаг сетки:"), grid_step_spin_);

    depth_label_step_spin_ = new QDoubleSpinBox(tab);
    depth_label_step_spin_->setRange(10, 10000);
    depth_label_step_spin_->setValue(500);
    depth_label_step_spin_->setSuffix(tr(" м"));
    size_layout->addRow(tr("Шаг отметок глубины:"), depth_label_step_spin_);

    layout->addWidget(size_group);
    layout->addStretch();
}

void ViewOptionsDialog::setupTrajectoryTab(QWidget* tab) {
    auto* layout = new QVBoxLayout(tab);

    auto* group = new QGroupBox(tr("Отображение траекторий"), tab);
    auto* form_layout = new QFormLayout(group);

    line_width_spin_ = new QDoubleSpinBox(tab);
    line_width_spin_->setRange(0.5, 10);
    line_width_spin_->setValue(2);
    line_width_spin_->setSingleStep(0.5);
    line_width_spin_->setSuffix(tr(" пикс."));
    form_layout->addRow(tr("Толщина линии:"), line_width_spin_);

    show_wellhead_check_ = new QCheckBox(tr("Показывать устье"), tab);
    form_layout->addRow(show_wellhead_check_);

    show_points_check_ = new QCheckBox(tr("Показывать точки замеров"), tab);
    form_layout->addRow(show_points_check_);

    layout->addWidget(group);
    layout->addStretch();
}

void ViewOptionsDialog::setupPointsTab(QWidget* tab) {
    auto* layout = new QVBoxLayout(tab);

    // Проектные точки
    auto* project_group = new QGroupBox(tr("Проектные точки"), tab);
    auto* project_layout = new QFormLayout(project_group);

    show_project_points_check_ = new QCheckBox(tr("Показывать проектные точки"), tab);
    project_layout->addRow(show_project_points_check_);

    show_tolerance_check_ = new QCheckBox(tr("Показывать круги допуска"), tab);
    project_layout->addRow(show_tolerance_check_);

    tolerance_alpha_spin_ = new QDoubleSpinBox(tab);
    tolerance_alpha_spin_->setRange(0, 1);
    tolerance_alpha_spin_->setValue(0.3);
    tolerance_alpha_spin_->setSingleStep(0.1);
    project_layout->addRow(tr("Прозрачность кругов:"), tolerance_alpha_spin_);

    layout->addWidget(project_group);

    // Пункты возбуждения
    auto* shot_group = new QGroupBox(tr("Пункты возбуждения"), tab);
    auto* shot_layout = new QFormLayout(shot_group);

    show_shot_points_check_ = new QCheckBox(tr("Показывать пункты возбуждения"), tab);
    shot_layout->addRow(show_shot_points_check_);

    shot_size_spin_ = new QDoubleSpinBox(tab);
    shot_size_spin_->setRange(1, 50);
    shot_size_spin_->setValue(10);
    shot_size_spin_->setSuffix(tr(" пикс."));
    shot_layout->addRow(tr("Размер маркера:"), shot_size_spin_);

    layout->addWidget(shot_group);
    layout->addStretch();
}

void ViewOptionsDialog::setup3DTab(QWidget* tab) {
    auto* layout = new QVBoxLayout(tab);

    auto* view_group = new QGroupBox(tr("3D отображение"), tab);
    auto* view_layout = new QFormLayout(view_group);

    show_sea_level_check_ = new QCheckBox(tr("Показывать уровень моря"), tab);
    view_layout->addRow(show_sea_level_check_);

    perspective_check_ = new QCheckBox(tr("Перспективная проекция"), tab);
    view_layout->addRow(perspective_check_);

    rotation_sens_spin_ = new QDoubleSpinBox(tab);
    rotation_sens_spin_->setRange(0.1, 5);
    rotation_sens_spin_->setValue(1.0);
    rotation_sens_spin_->setSingleStep(0.1);
    view_layout->addRow(tr("Чувствительность вращения:"), rotation_sens_spin_);

    layout->addWidget(view_group);

    // Вертикальная проекция
    auto* vertical_group = new QGroupBox(tr("Вертикальная проекция"), tab);
    auto* vertical_layout = new QFormLayout(vertical_group);

    auto_azimuth_check_ = new QCheckBox(tr("Автоподбор азимута"), tab);
    vertical_layout->addRow(auto_azimuth_check_);

    azimuth_spin_ = new QDoubleSpinBox(tab);
    azimuth_spin_->setRange(0, 360);
    azimuth_spin_->setValue(0);
    azimuth_spin_->setSuffix(tr("°"));
    vertical_layout->addRow(tr("Азимут профиля:"), azimuth_spin_);

    layout->addWidget(vertical_group);

    // Связь между автоподбором и ручным вводом
    connect(auto_azimuth_check_, &QCheckBox::toggled,
            azimuth_spin_, &QDoubleSpinBox::setDisabled);

    layout->addStretch();
}

void ViewOptionsDialog::setupColorsTab(QWidget* tab) {
    auto* layout = new QVBoxLayout(tab);

    auto* group = new QGroupBox(tr("Цвета"), tab);
    auto* form_layout = new QFormLayout(group);

    bg_color_button_ = new QPushButton(tab);
    bg_color_button_->setMinimumWidth(100);
    connect(bg_color_button_, &QPushButton::clicked, this, &ViewOptionsDialog::onColorButton);
    form_layout->addRow(tr("Фон:"), bg_color_button_);

    grid_color_button_ = new QPushButton(tab);
    grid_color_button_->setMinimumWidth(100);
    connect(grid_color_button_, &QPushButton::clicked, this, &ViewOptionsDialog::onColorButton);
    form_layout->addRow(tr("Сетка:"), grid_color_button_);

    axis_x_button_ = new QPushButton(tab);
    axis_x_button_->setMinimumWidth(100);
    connect(axis_x_button_, &QPushButton::clicked, this, &ViewOptionsDialog::onColorButton);
    form_layout->addRow(tr("Ось X (восток):"), axis_x_button_);

    axis_y_button_ = new QPushButton(tab);
    axis_y_button_->setMinimumWidth(100);
    connect(axis_y_button_, &QPushButton::clicked, this, &ViewOptionsDialog::onColorButton);
    form_layout->addRow(tr("Ось Y (север):"), axis_y_button_);

    axis_z_button_ = new QPushButton(tab);
    axis_z_button_->setMinimumWidth(100);
    connect(axis_z_button_, &QPushButton::clicked, this, &ViewOptionsDialog::onColorButton);
    form_layout->addRow(tr("Ось Z (глубина):"), axis_z_button_);

    sea_level_button_ = new QPushButton(tab);
    sea_level_button_->setMinimumWidth(100);
    connect(sea_level_button_, &QPushButton::clicked, this, &ViewOptionsDialog::onColorButton);
    form_layout->addRow(tr("Уровень моря:"), sea_level_button_);

    layout->addWidget(group);
    layout->addStretch();

    // Установка начальных цветов
    updateColorButton(bg_color_button_, Qt::white);
    updateColorButton(grid_color_button_, QColor(200, 200, 200));
    updateColorButton(axis_x_button_, Qt::red);
    updateColorButton(axis_y_button_, Qt::green);
    updateColorButton(axis_z_button_, Qt::blue);
    updateColorButton(sea_level_button_, QColor(0, 100, 200, 80));

    bg_color_ = Qt::white;
    grid_color_ = QColor(200, 200, 200);
    axis_x_color_ = Qt::red;
    axis_y_color_ = Qt::green;
    axis_z_color_ = Qt::blue;
    sea_level_color_ = QColor(0, 100, 200, 80);
}

void ViewOptionsDialog::setOptions(const ViewOptions& options) {
    show_grid_check_->setChecked(options.show_grid);
    show_axes_check_->setChecked(options.show_axes);
    show_labels_check_->setChecked(options.show_labels);
    show_depth_marks_check_->setChecked(options.show_depth_marks);
    grid_step_spin_->setValue(options.grid_step);
    depth_label_step_spin_->setValue(options.depth_label_step);

    line_width_spin_->setValue(options.default_line_width);
    show_wellhead_check_->setChecked(options.show_wellhead);
    show_points_check_->setChecked(options.show_trajectory_points);

    show_project_points_check_->setChecked(options.show_project_points);
    show_tolerance_check_->setChecked(options.show_tolerance_circles);
    tolerance_alpha_spin_->setValue(options.tolerance_circle_alpha);
    show_shot_points_check_->setChecked(options.show_shot_points);
    shot_size_spin_->setValue(options.shot_point_size);

    show_sea_level_check_->setChecked(options.show_sea_level_plane);
    perspective_check_->setChecked(options.enable_perspective);
    rotation_sens_spin_->setValue(options.rotation_sensitivity);

    auto_azimuth_check_->setChecked(options.auto_fit_azimuth);
    azimuth_spin_->setValue(options.profile_azimuth);

    bg_color_ = options.background_color;
    grid_color_ = options.grid_color;
    axis_x_color_ = options.axis_x_color;
    axis_y_color_ = options.axis_y_color;
    axis_z_color_ = options.axis_z_color;
    sea_level_color_ = options.sea_level_color;

    updateColorButton(bg_color_button_, bg_color_);
    updateColorButton(grid_color_button_, grid_color_);
    updateColorButton(axis_x_button_, axis_x_color_);
    updateColorButton(axis_y_button_, axis_y_color_);
    updateColorButton(axis_z_button_, axis_z_color_);
    updateColorButton(sea_level_button_, sea_level_color_);
}

ViewOptions ViewOptionsDialog::options() const {
    ViewOptions opts;

    opts.show_grid = show_grid_check_->isChecked();
    opts.show_axes = show_axes_check_->isChecked();
    opts.show_labels = show_labels_check_->isChecked();
    opts.show_depth_marks = show_depth_marks_check_->isChecked();
    opts.grid_step = grid_step_spin_->value();
    opts.depth_label_step = depth_label_step_spin_->value();

    opts.default_line_width = line_width_spin_->value();
    opts.show_wellhead = show_wellhead_check_->isChecked();
    opts.show_trajectory_points = show_points_check_->isChecked();

    opts.show_project_points = show_project_points_check_->isChecked();
    opts.show_tolerance_circles = show_tolerance_check_->isChecked();
    opts.tolerance_circle_alpha = tolerance_alpha_spin_->value();
    opts.show_shot_points = show_shot_points_check_->isChecked();
    opts.shot_point_size = shot_size_spin_->value();

    opts.show_sea_level_plane = show_sea_level_check_->isChecked();
    opts.enable_perspective = perspective_check_->isChecked();
    opts.rotation_sensitivity = rotation_sens_spin_->value();

    opts.auto_fit_azimuth = auto_azimuth_check_->isChecked();
    opts.profile_azimuth = azimuth_spin_->value();

    opts.background_color = bg_color_;
    opts.grid_color = grid_color_;
    opts.axis_x_color = axis_x_color_;
    opts.axis_y_color = axis_y_color_;
    opts.axis_z_color = axis_z_color_;
    opts.sea_level_color = sea_level_color_;

    return opts;
}

void ViewOptionsDialog::onApply() {
    emit optionsChanged(options());
}

void ViewOptionsDialog::onColorButton() {
    QPushButton* button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    QColor* color = nullptr;
    if (button == bg_color_button_) color = &bg_color_;
    else if (button == grid_color_button_) color = &grid_color_;
    else if (button == axis_x_button_) color = &axis_x_color_;
    else if (button == axis_y_button_) color = &axis_y_color_;
    else if (button == axis_z_button_) color = &axis_z_color_;
    else if (button == sea_level_button_) color = &sea_level_color_;

    if (!color) return;

    QColorDialog dialog(*color, this);
    dialog.setOption(QColorDialog::ShowAlphaChannel, button == sea_level_button_);

    if (dialog.exec() == QDialog::Accepted) {
        *color = dialog.currentColor();
        updateColorButton(button, *color);
    }
}

void ViewOptionsDialog::updateColorButton(QPushButton* button, const QColor& color) {
    QString style = QString("QPushButton { background-color: %1; }").arg(color.name());
    button->setStyleSheet(style);
    button->setText(color.name());
}

}  // namespace incline3d::ui

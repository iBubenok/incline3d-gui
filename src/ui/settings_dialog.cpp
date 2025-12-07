#include "ui/settings_dialog.h"
#include "core/settings.h"

#include <QCheckBox>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QLineEdit>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

namespace incline3d::ui {

SettingsDialog::SettingsDialog(QWidget* parent) : QDialog(parent) {
    setWindowTitle(tr("Настройки"));
    setMinimumWidth(450);
    setupUi();
    loadSettings();
}

void SettingsDialog::setupUi() {
    auto* main_layout = new QVBoxLayout(this);

    // Группа путей
    auto* paths_group = new QGroupBox(tr("Пути"));
    auto* paths_layout = new QFormLayout(paths_group);

    auto* path_widget = new QWidget();
    auto* path_layout = new QHBoxLayout(path_widget);
    path_layout->setContentsMargins(0, 0, 0, 0);

    inclproc_path_edit_ = new QLineEdit();
    path_layout->addWidget(inclproc_path_edit_);

    auto* browse_btn = new QPushButton(tr("..."));
    browse_btn->setMaximumWidth(30);
    connect(browse_btn, &QPushButton::clicked, this, &SettingsDialog::onBrowseInclproc);
    path_layout->addWidget(browse_btn);

    paths_layout->addRow(tr("Путь к inclproc:"), path_widget);
    main_layout->addWidget(paths_group);

    // Группа автосохранения
    auto* autosave_group = new QGroupBox(tr("Автосохранение"));
    auto* autosave_layout = new QFormLayout(autosave_group);

    autosave_enabled_check_ = new QCheckBox(tr("Включить автосохранение"));
    autosave_layout->addRow(autosave_enabled_check_);

    autosave_interval_spin_ = new QSpinBox();
    autosave_interval_spin_->setRange(1, 60);
    autosave_interval_spin_->setSuffix(tr(" мин"));
    autosave_layout->addRow(tr("Интервал:"), autosave_interval_spin_);

    main_layout->addWidget(autosave_group);

    main_layout->addStretch();

    // Кнопки
    auto* buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(buttons, &QDialogButtonBox::accepted, this, &SettingsDialog::onAccept);
    connect(buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    main_layout->addWidget(buttons);
}

void SettingsDialog::loadSettings() {
    auto& s = core::Settings::instance();
    inclproc_path_edit_->setText(s.inclprocPath());
    autosave_enabled_check_->setChecked(s.autoSaveEnabled());
    autosave_interval_spin_->setValue(s.autoSaveIntervalMinutes());
}

void SettingsDialog::onBrowseInclproc() {
    QString path = QFileDialog::getOpenFileName(this, tr("Выберите inclproc"),
                                                inclproc_path_edit_->text());
    if (!path.isEmpty()) {
        inclproc_path_edit_->setText(path);
    }
}

void SettingsDialog::onAccept() {
    auto& s = core::Settings::instance();
    s.setInclprocPath(inclproc_path_edit_->text());
    s.setAutoSaveEnabled(autosave_enabled_check_->isChecked());
    s.setAutoSaveIntervalMinutes(autosave_interval_spin_->value());
    s.save();
    accept();
}

}  // namespace incline3d::ui

#include "ui/proximity_dialog.h"
#include "models/well_table_model.h"
#include "core/incline_process_runner.h"

#include <QComboBox>
#include <QDialogButtonBox>
#include <QDoubleSpinBox>
#include <QFormLayout>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>

namespace incline3d::ui {

ProximityDialog::ProximityDialog(models::WellTableModel* model,
                                 core::InclineProcessRunner* runner,
                                 QWidget* parent)
    : QDialog(parent), model_(model), runner_(runner) {
    setWindowTitle(tr("Анализ сближения"));
    setMinimumWidth(400);

    auto* layout = new QVBoxLayout(this);
    auto* form = new QFormLayout();

    well_a_combo_ = new QComboBox();
    well_b_combo_ = new QComboBox();

    for (int i = 0; i < model_->wellCount(); ++i) {
        auto well = model_->wellAt(i);
        QString name = QString::fromStdString(well->metadata.well_name);
        well_a_combo_->addItem(name, i);
        well_b_combo_->addItem(name, i);
    }

    if (model_->wellCount() >= 2) {
        well_b_combo_->setCurrentIndex(1);
    }

    form->addRow(tr("Скважина A:"), well_a_combo_);
    form->addRow(tr("Скважина B:"), well_b_combo_);

    tolerance_spin_ = new QDoubleSpinBox();
    tolerance_spin_->setRange(0, 1000);
    tolerance_spin_->setValue(5);
    tolerance_spin_->setSuffix(tr(" м"));
    form->addRow(tr("Допуск:"), tolerance_spin_);

    layout->addLayout(form);

    result_label_ = new QLabel();
    result_label_->setStyleSheet("font-weight: bold; font-size: 14px;");
    result_label_->setAlignment(Qt::AlignCenter);
    layout->addWidget(result_label_);

    layout->addStretch();

    auto* btn_layout = new QHBoxLayout();
    auto* calc_btn = new QPushButton(tr("Рассчитать"));
    connect(calc_btn, &QPushButton::clicked, this, &ProximityDialog::onCalculate);
    btn_layout->addWidget(calc_btn);

    auto* close_btn = new QPushButton(tr("Закрыть"));
    connect(close_btn, &QPushButton::clicked, this, &QDialog::accept);
    btn_layout->addWidget(close_btn);

    layout->addLayout(btn_layout);
}

void ProximityDialog::onCalculate() {
    int idx_a = well_a_combo_->currentData().toInt();
    int idx_b = well_b_combo_->currentData().toInt();

    if (idx_a == idx_b) {
        result_label_->setText(tr("Выберите разные скважины"));
        return;
    }

    // Упрощённая заглушка - для реальной работы нужно вызвать inclproc proximity
    result_label_->setText(tr("Минимальная дистанция: — м\n"
                              "(требуется интеграция с inclproc)"));
}

}  // namespace incline3d::ui

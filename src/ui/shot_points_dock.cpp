#include "ui/shot_points_dock.h"
#include "models/shot_points_model.h"
#include <QVBoxLayout>
#include <QHeaderView>

namespace incline3d::ui {

ShotPointsDock::ShotPointsDock(models::ShotPointsModel* model, QWidget* parent)
    : QDockWidget(tr("Пункты возбуждения"), parent) {
    auto* widget = new QWidget(this);
    auto* layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);

    table_view_ = new QTableView(widget);
    table_view_->setModel(model);
    table_view_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_view_->setAlternatingRowColors(true);
    table_view_->horizontalHeader()->setStretchLastSection(true);
    table_view_->verticalHeader()->setDefaultSectionSize(24);

    layout->addWidget(table_view_);
    setWidget(widget);
}

}  // namespace incline3d::ui

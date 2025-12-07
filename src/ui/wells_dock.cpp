#include "ui/wells_dock.h"
#include "models/well_table_model.h"

#include <QHeaderView>
#include <QVBoxLayout>

namespace incline3d::ui {

WellsDock::WellsDock(models::WellTableModel* model, QWidget* parent)
    : QDockWidget(tr("Скважины"), parent), model_(model) {

    auto* widget = new QWidget(this);
    auto* layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);

    table_view_ = new QTableView(widget);
    table_view_->setModel(model_);
    table_view_->setSelectionBehavior(QAbstractItemView::SelectRows);
    table_view_->setSelectionMode(QAbstractItemView::SingleSelection);
    table_view_->setAlternatingRowColors(true);
    table_view_->horizontalHeader()->setStretchLastSection(true);
    table_view_->verticalHeader()->setDefaultSectionSize(24);

    // Скрываем некоторые колонки для компактности
    table_view_->setColumnHidden(models::WellTableModel::kColumnField, true);
    table_view_->setColumnHidden(models::WellTableModel::kColumnCluster, true);

    layout->addWidget(table_view_);
    setWidget(widget);

    connect(table_view_->selectionModel(), &QItemSelectionModel::selectionChanged,
            this, &WellsDock::onSelectionChanged);
    connect(table_view_, &QTableView::doubleClicked,
            this, &WellsDock::onDoubleClicked);
}

void WellsDock::onSelectionChanged() {
    auto indexes = table_view_->selectionModel()->selectedRows();
    if (!indexes.isEmpty()) {
        emit wellSelected(indexes.first().row());
    }
}

void WellsDock::onDoubleClicked(const QModelIndex& index) {
    emit wellSelected(index.row());
}

}  // namespace incline3d::ui

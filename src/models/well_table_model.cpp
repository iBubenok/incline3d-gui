#include "models/well_table_model.h"

#include <QBrush>
#include <QFont>

namespace incline3d::models {

WellTableModel::WellTableModel(QObject* parent)
    : QAbstractTableModel(parent) {
}

int WellTableModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return static_cast<int>(wells_.size());
}

int WellTableModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return kColumnCount;
}

QVariant WellTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(wells_.size())) {
        return {};
    }

    const auto& well = wells_[index.row()];

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
            case kColumnVisible:
                return {};  // Чекбокс обрабатывается через CheckStateRole
            case kColumnName:
                return QString::fromStdString(well->metadata.well_name);
            case kColumnField:
                return QString::fromStdString(well->metadata.field_name);
            case kColumnCluster:
                return QString::fromStdString(well->metadata.well_pad);
            case kColumnDepth:
                return well->total_depth > 0 ? QString::number(well->total_depth, 'f', 1) : QString();
            case kColumnMaxAngle:
                return well->max_inclination_deg > 0
                           ? QString::number(well->max_inclination_deg, 'f', 2)
                           : QString();
            case kColumnMaxIntensity:
                return well->max_intensity_10m > 0
                           ? QString::number(well->max_intensity_10m, 'f', 2)
                           : QString();
            case kColumnDisplacement:
                return well->horizontal_displacement > 0
                           ? QString::number(well->horizontal_displacement, 'f', 1)
                           : QString();
            case kColumnColor:
                return {};  // Цвет отображается через DecorationRole
        }
    }

    if (role == Qt::CheckStateRole && index.column() == kColumnVisible) {
        return well->visible ? Qt::Checked : Qt::Unchecked;
    }

    if (role == Qt::DecorationRole && index.column() == kColumnColor) {
        return well->display_color;
    }

    if (role == Qt::BackgroundRole && index.column() == kColumnColor) {
        return QBrush(well->display_color);
    }

    if (role == Qt::ToolTipRole) {
        switch (index.column()) {
            case kColumnName:
                return QString::fromStdString(well->source_file_path);
            case kColumnDepth:
                return tr("Забой по стволу, м");
            case kColumnMaxAngle:
                return tr("Максимальный угол наклона, °");
            case kColumnMaxIntensity:
                return tr("Максимальная интенсивность на 10 м, °/10м");
            case kColumnDisplacement:
                return tr("Горизонтальное смещение забоя, м");
        }
    }

    if (role == Qt::FontRole && well->modified) {
        QFont font;
        font.setBold(true);
        return font;
    }

    return {};
}

QVariant WellTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return {};
    }

    switch (section) {
        case kColumnVisible:
            return tr("Вид");
        case kColumnName:
            return tr("Скважина");
        case kColumnField:
            return tr("Месторождение");
        case kColumnCluster:
            return tr("Куст");
        case kColumnDepth:
            return tr("Глубина, м");
        case kColumnMaxAngle:
            return tr("Макс. угол, °");
        case kColumnMaxIntensity:
            return tr("Макс. инт., °/10м");
        case kColumnDisplacement:
            return tr("Смещение, м");
        case kColumnColor:
            return tr("Цвет");
    }

    return {};
}

Qt::ItemFlags WellTableModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (index.column() == kColumnVisible) {
        flags |= Qt::ItemIsUserCheckable;
    }

    if (index.column() == kColumnName || index.column() == kColumnColor) {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}

bool WellTableModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid() || index.row() >= static_cast<int>(wells_.size())) {
        return false;
    }

    auto& well = wells_[index.row()];

    if (role == Qt::CheckStateRole && index.column() == kColumnVisible) {
        well->visible = (value.toInt() == Qt::Checked);
        emit dataChanged(index, index, {role});
        emit wellVisibilityChanged(index.row(), well->visible);
        return true;
    }

    if (role == Qt::EditRole) {
        switch (index.column()) {
            case kColumnName:
                well->metadata.well_name = value.toString().toStdString();
                well->modified = true;
                emit dataChanged(index, index, {role});
                emit wellDataChanged(index.row());
                return true;
            case kColumnColor:
                if (value.canConvert<QColor>()) {
                    well->display_color = value.value<QColor>();
                    emit dataChanged(index, index, {Qt::DecorationRole, Qt::BackgroundRole});
                    emit wellColorChanged(index.row(), well->display_color);
                    return true;
                }
                break;
        }
    }

    return false;
}

void WellTableModel::addWell(std::shared_ptr<WellData> well) {
    int row = static_cast<int>(wells_.size());
    beginInsertRows(QModelIndex(), row, row);
    wells_.push_back(std::move(well));
    endInsertRows();
}

void WellTableModel::removeWell(int index) {
    if (index < 0 || index >= static_cast<int>(wells_.size())) {
        return;
    }
    beginRemoveRows(QModelIndex(), index, index);
    wells_.erase(wells_.begin() + index);
    endRemoveRows();
}

void WellTableModel::clear() {
    if (wells_.empty()) {
        return;
    }
    beginResetModel();
    wells_.clear();
    endResetModel();
}

std::shared_ptr<WellData> WellTableModel::wellAt(int index) const {
    if (index < 0 || index >= static_cast<int>(wells_.size())) {
        return nullptr;
    }
    return wells_[index];
}

int WellTableModel::wellCount() const {
    return static_cast<int>(wells_.size());
}

std::vector<std::shared_ptr<WellData>>& WellTableModel::wells() {
    return wells_;
}

const std::vector<std::shared_ptr<WellData>>& WellTableModel::wells() const {
    return wells_;
}

int WellTableModel::findWellByName(const std::string& name) const {
    for (size_t i = 0; i < wells_.size(); ++i) {
        if (wells_[i]->metadata.well_name == name) {
            return static_cast<int>(i);
        }
    }
    return -1;
}

void WellTableModel::updateWell(int index) {
    if (index < 0 || index >= static_cast<int>(wells_.size())) {
        return;
    }
    emit dataChanged(createIndex(index, 0), createIndex(index, kColumnCount - 1));
    emit wellDataChanged(index);
}

}  // namespace incline3d::models

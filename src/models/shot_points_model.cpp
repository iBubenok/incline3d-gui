#include "models/shot_points_model.h"

#include <QBrush>

namespace incline3d::models {

ShotPointsModel::ShotPointsModel(QObject* parent)
    : QAbstractTableModel(parent) {
}

int ShotPointsModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return static_cast<int>(points_.size());
}

int ShotPointsModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return kColumnCount;
}

QVariant ShotPointsModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || index.row() >= static_cast<int>(points_.size())) {
        return {};
    }

    const auto& point = points_[index.row()];

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
            case kColumnVisible:
                return {};
            case kColumnName:
                return QString::fromStdString(point.name);
            case kColumnX:
                return QString::number(point.x_m, 'f', 1);
            case kColumnY:
                return QString::number(point.y_m, 'f', 1);
            case kColumnZ:
                return QString::number(point.z_m, 'f', 1);
            case kColumnMarker:
                return QString::fromStdString(marker_to_string(point.marker));
            case kColumnColor:
                return {};
        }
    }

    if (role == Qt::CheckStateRole && index.column() == kColumnVisible) {
        return point.visible ? Qt::Checked : Qt::Unchecked;
    }

    if (role == Qt::DecorationRole && index.column() == kColumnColor) {
        return point.display_color;
    }

    if (role == Qt::BackgroundRole && index.column() == kColumnColor) {
        return QBrush(point.display_color);
    }

    if (role == Qt::ToolTipRole) {
        switch (index.column()) {
            case kColumnX:
                return tr("Координата X (восток), м");
            case kColumnY:
                return tr("Координата Y (север), м");
            case kColumnZ:
                return tr("Координата Z (глубина/высота), м");
            case kColumnMarker:
                return tr("Тип маркера");
        }
    }

    return {};
}

QVariant ShotPointsModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return {};
    }

    switch (section) {
        case kColumnVisible:
            return tr("Вид");
        case kColumnName:
            return tr("Название");
        case kColumnX:
            return tr("X, м");
        case kColumnY:
            return tr("Y, м");
        case kColumnZ:
            return tr("Z, м");
        case kColumnMarker:
            return tr("Маркер");
        case kColumnColor:
            return tr("Цвет");
    }

    return {};
}

Qt::ItemFlags ShotPointsModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (index.column() == kColumnVisible) {
        flags |= Qt::ItemIsUserCheckable;
    }

    // Редактируемые колонки
    if (index.column() != kColumnVisible) {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}

bool ShotPointsModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid() || index.row() >= static_cast<int>(points_.size())) {
        return false;
    }

    auto& point = points_[index.row()];

    if (role == Qt::CheckStateRole && index.column() == kColumnVisible) {
        point.visible = (value.toInt() == Qt::Checked);
        emit dataChanged(index, index, {role});
        emit pointVisibilityChanged(index.row(), point.visible);
        return true;
    }

    if (role == Qt::EditRole) {
        bool ok = false;
        switch (index.column()) {
            case kColumnName:
                point.name = value.toString().toStdString();
                emit dataChanged(index, index, {role});
                emit pointDataChanged(index.row());
                return true;
            case kColumnX:
                point.x_m = value.toDouble(&ok);
                if (ok) {
                    emit dataChanged(index, index, {role});
                    emit pointDataChanged(index.row());
                    return true;
                }
                break;
            case kColumnY:
                point.y_m = value.toDouble(&ok);
                if (ok) {
                    emit dataChanged(index, index, {role});
                    emit pointDataChanged(index.row());
                    return true;
                }
                break;
            case kColumnZ:
                point.z_m = value.toDouble(&ok);
                if (ok) {
                    emit dataChanged(index, index, {role});
                    emit pointDataChanged(index.row());
                    return true;
                }
                break;
            case kColumnMarker:
                point.marker = string_to_marker(value.toString().toStdString());
                emit dataChanged(index, index, {role});
                emit pointDataChanged(index.row());
                return true;
            case kColumnColor:
                if (value.canConvert<QColor>()) {
                    point.display_color = value.value<QColor>();
                    emit dataChanged(index, index, {Qt::DecorationRole, Qt::BackgroundRole});
                    emit pointDataChanged(index.row());
                    return true;
                }
                break;
        }
    }

    return false;
}

void ShotPointsModel::addPoint(const ShotPoint& point) {
    int row = static_cast<int>(points_.size());
    beginInsertRows(QModelIndex(), row, row);
    points_.push_back(point);
    endInsertRows();
}

void ShotPointsModel::removePoint(int index) {
    if (index < 0 || index >= static_cast<int>(points_.size())) {
        return;
    }
    beginRemoveRows(QModelIndex(), index, index);
    points_.erase(points_.begin() + index);
    endRemoveRows();
}

void ShotPointsModel::clear() {
    if (points_.empty()) {
        return;
    }
    beginResetModel();
    points_.clear();
    endResetModel();
}

void ShotPointsModel::setPoints(const std::vector<ShotPoint>& points) {
    beginResetModel();
    points_ = points;
    endResetModel();
}

ShotPoint& ShotPointsModel::pointAt(int index) {
    return points_.at(index);
}

const ShotPoint& ShotPointsModel::pointAt(int index) const {
    return points_.at(index);
}

int ShotPointsModel::pointCount() const {
    return static_cast<int>(points_.size());
}

std::vector<ShotPoint>& ShotPointsModel::points() {
    return points_;
}

const std::vector<ShotPoint>& ShotPointsModel::points() const {
    return points_;
}

}  // namespace incline3d::models

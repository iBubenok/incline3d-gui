#include "models/project_points_model.h"

#include <QBrush>

namespace incline3d::models {

ProjectPointsModel::ProjectPointsModel(QObject* parent)
    : QAbstractTableModel(parent) {
}

int ProjectPointsModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return static_cast<int>(points_.size());
}

int ProjectPointsModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return kColumnCount;
}

QVariant ProjectPointsModel::data(const QModelIndex& index, int role) const {
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
            case kColumnAzimuth:
                return QString::number(point.azimuth_geogr_deg, 'f', 2);
            case kColumnShift:
                return QString::number(point.shift_m, 'f', 1);
            case kColumnDepth:
                return QString::number(point.depth_m, 'f', 1);
            case kColumnAbsDepth:
                return QString::number(point.abs_depth_m, 'f', 1);
            case kColumnRadius:
                return QString::number(point.radius_m, 'f', 1);
            case kColumnFactOffset:
                return point.fact_offset_m != 0.0
                           ? QString::number(point.fact_offset_m, 'f', 1)
                           : QString();
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
            case kColumnAzimuth:
                return tr("Географический азимут, °");
            case kColumnShift:
                return tr("Горизонтальное смещение от устья, м");
            case kColumnDepth:
                return tr("Глубина по стволу, м");
            case kColumnAbsDepth:
                return tr("Абсолютная глубина, м");
            case kColumnRadius:
                return tr("Радиус допуска, м");
            case kColumnFactOffset:
                return tr("Фактическое отклонение от проекта, м");
        }
    }

    // Подсветка при выходе за допуск
    if (role == Qt::BackgroundRole && index.column() == kColumnFactOffset) {
        if (point.radius_m > 0 && point.fact_offset_m > point.radius_m) {
            return QBrush(Qt::red);
        }
    }

    return {};
}

QVariant ProjectPointsModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return {};
    }

    switch (section) {
        case kColumnVisible:
            return tr("Вид");
        case kColumnName:
            return tr("Пласт");
        case kColumnAzimuth:
            return tr("Азимут, °");
        case kColumnShift:
            return tr("Смещ., м");
        case kColumnDepth:
            return tr("Глуб., м");
        case kColumnAbsDepth:
            return tr("Абс., м");
        case kColumnRadius:
            return tr("R, м");
        case kColumnFactOffset:
            return tr("Откл., м");
        case kColumnColor:
            return tr("Цвет");
    }

    return {};
}

Qt::ItemFlags ProjectPointsModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    Qt::ItemFlags flags = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    if (index.column() == kColumnVisible) {
        flags |= Qt::ItemIsUserCheckable;
    }

    // Редактируемые колонки
    if (index.column() == kColumnName || index.column() == kColumnAzimuth ||
        index.column() == kColumnShift || index.column() == kColumnDepth ||
        index.column() == kColumnAbsDepth || index.column() == kColumnRadius ||
        index.column() == kColumnColor) {
        flags |= Qt::ItemIsEditable;
    }

    return flags;
}

bool ProjectPointsModel::setData(const QModelIndex& index, const QVariant& value, int role) {
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
            case kColumnAzimuth:
                point.azimuth_geogr_deg = value.toDouble(&ok);
                if (ok) {
                    emit dataChanged(index, index, {role});
                    emit pointDataChanged(index.row());
                    return true;
                }
                break;
            case kColumnShift:
                point.shift_m = value.toDouble(&ok);
                if (ok) {
                    emit dataChanged(index, index, {role});
                    emit pointDataChanged(index.row());
                    return true;
                }
                break;
            case kColumnDepth:
                point.depth_m = value.toDouble(&ok);
                if (ok) {
                    emit dataChanged(index, index, {role});
                    emit pointDataChanged(index.row());
                    return true;
                }
                break;
            case kColumnAbsDepth:
                point.abs_depth_m = value.toDouble(&ok);
                if (ok) {
                    emit dataChanged(index, index, {role});
                    emit pointDataChanged(index.row());
                    return true;
                }
                break;
            case kColumnRadius:
                point.radius_m = value.toDouble(&ok);
                if (ok) {
                    emit dataChanged(index, index, {role});
                    emit pointDataChanged(index.row());
                    return true;
                }
                break;
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

void ProjectPointsModel::addPoint(const ProjectPoint& point) {
    int row = static_cast<int>(points_.size());
    beginInsertRows(QModelIndex(), row, row);
    points_.push_back(point);
    endInsertRows();
}

void ProjectPointsModel::removePoint(int index) {
    if (index < 0 || index >= static_cast<int>(points_.size())) {
        return;
    }
    beginRemoveRows(QModelIndex(), index, index);
    points_.erase(points_.begin() + index);
    endRemoveRows();
}

void ProjectPointsModel::clear() {
    if (points_.empty()) {
        return;
    }
    beginResetModel();
    points_.clear();
    endResetModel();
}

void ProjectPointsModel::setPoints(const std::vector<ProjectPoint>& points) {
    beginResetModel();
    points_ = points;
    endResetModel();
}

ProjectPoint& ProjectPointsModel::pointAt(int index) {
    return points_.at(index);
}

const ProjectPoint& ProjectPointsModel::pointAt(int index) const {
    return points_.at(index);
}

int ProjectPointsModel::pointCount() const {
    return static_cast<int>(points_.size());
}

std::vector<ProjectPoint>& ProjectPointsModel::points() {
    return points_;
}

const std::vector<ProjectPoint>& ProjectPointsModel::points() const {
    return points_;
}

void ProjectPointsModel::updateFactValues(int index) {
    if (index < 0 || index >= static_cast<int>(points_.size())) {
        return;
    }
    emit dataChanged(createIndex(index, 0), createIndex(index, kColumnCount - 1));
}

}  // namespace incline3d::models

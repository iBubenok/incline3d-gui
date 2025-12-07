#include "models/measurements_model.h"

namespace incline3d::models {

MeasurementsModel::MeasurementsModel(QObject* parent)
    : QAbstractTableModel(parent) {
}

int MeasurementsModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid() || !well_) {
        return 0;
    }
    return static_cast<int>(well_->measurements.size());
}

int MeasurementsModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return kColumnCount;
}

QVariant MeasurementsModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || !well_ ||
        index.row() >= static_cast<int>(well_->measurements.size())) {
        return {};
    }

    const auto& point = well_->measurements[index.row()];

    if (role == Qt::DisplayRole || role == Qt::EditRole) {
        switch (index.column()) {
            case kColumnDepth:
                return QString::number(point.measured_depth_m, 'f', 2);
            case kColumnInclination:
                return QString::number(point.inclination_deg, 'f', 2);
            case kColumnAzimuth:
                if (point.azimuth_deg.has_value()) {
                    return QString::number(point.azimuth_deg.value(), 'f', 2);
                }
                return QString();
            case kColumnAzimuthType:
                switch (point.azimuth_type) {
                    case AzimuthType::kMagnetic: return tr("Магн.");
                    case AzimuthType::kTrue: return tr("Истин.");
                    case AzimuthType::kGrid: return tr("Дир.");
                }
                return tr("Магн.");
        }
    }

    if (role == Qt::ToolTipRole) {
        switch (index.column()) {
            case kColumnDepth:
                return tr("Глубина по стволу, м");
            case kColumnInclination:
                return tr("Угол наклона от вертикали, °");
            case kColumnAzimuth:
                return tr("Азимут, ° (пусто = отсутствует)");
            case kColumnAzimuthType:
                return tr("Тип азимута: магнитный или истинный");
        }
    }

    if (role == Qt::TextAlignmentRole) {
        return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
    }

    return {};
}

QVariant MeasurementsModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return {};
    }

    switch (section) {
        case kColumnDepth:
            return tr("Глубина, м");
        case kColumnInclination:
            return tr("Угол, °");
        case kColumnAzimuth:
            return tr("Азимут, °");
        case kColumnAzimuthType:
            return tr("Тип");
    }

    return {};
}

Qt::ItemFlags MeasurementsModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    return Qt::ItemIsEnabled | Qt::ItemIsSelectable | Qt::ItemIsEditable;
}

bool MeasurementsModel::setData(const QModelIndex& index, const QVariant& value, int role) {
    if (!index.isValid() || !well_ || role != Qt::EditRole ||
        index.row() >= static_cast<int>(well_->measurements.size())) {
        return false;
    }

    auto& point = well_->measurements[index.row()];
    bool ok = false;

    switch (index.column()) {
        case kColumnDepth: {
            double val = value.toDouble(&ok);
            if (ok && val >= 0) {
                point.measured_depth_m = val;
                well_->modified = true;
                emit dataChanged(index, index, {role});
                emit dataModified();
                return true;
            }
            break;
        }
        case kColumnInclination: {
            double val = value.toDouble(&ok);
            if (ok && val >= 0 && val <= 180) {
                point.inclination_deg = val;
                well_->modified = true;
                emit dataChanged(index, index, {role});
                emit dataModified();
                return true;
            }
            break;
        }
        case kColumnAzimuth: {
            QString str = value.toString().trimmed();
            if (str.isEmpty()) {
                point.azimuth_deg = std::nullopt;
                well_->modified = true;
                emit dataChanged(index, index, {role});
                emit dataModified();
                return true;
            }
            double val = str.toDouble(&ok);
            if (ok) {
                // Нормализация к [0, 360)
                while (val < 0) val += 360;
                while (val >= 360) val -= 360;
                point.azimuth_deg = val;
                well_->modified = true;
                emit dataChanged(index, index, {role});
                emit dataModified();
                return true;
            }
            break;
        }
        case kColumnAzimuthType: {
            QString str = value.toString().toLower();
            if (str.contains("магн") || str == "m" || str == "mag") {
                point.azimuth_type = AzimuthType::kMagnetic;
            } else if (str.contains("дир") || str == "g" || str == "grid") {
                point.azimuth_type = AzimuthType::kGrid;
            } else {
                point.azimuth_type = AzimuthType::kTrue;
            }
            well_->modified = true;
            emit dataChanged(index, index, {role});
            emit dataModified();
            return true;
        }
    }

    return false;
}

void MeasurementsModel::setWell(std::shared_ptr<WellData> well) {
    beginResetModel();
    well_ = std::move(well);
    endResetModel();
}

void MeasurementsModel::clearWell() {
    beginResetModel();
    well_.reset();
    endResetModel();
}

void MeasurementsModel::addPoint(const MeasuredPoint& point) {
    if (!well_) {
        return;
    }
    int row = static_cast<int>(well_->measurements.size());
    beginInsertRows(QModelIndex(), row, row);
    well_->measurements.push_back(point);
    well_->modified = true;
    endInsertRows();
    emit dataModified();
}

void MeasurementsModel::removePoint(int index) {
    if (!well_ || index < 0 || index >= static_cast<int>(well_->measurements.size())) {
        return;
    }
    beginRemoveRows(QModelIndex(), index, index);
    well_->measurements.erase(well_->measurements.begin() + index);
    well_->modified = true;
    endRemoveRows();
    emit dataModified();
}

void MeasurementsModel::insertPoint(int index, const MeasuredPoint& point) {
    if (!well_) {
        return;
    }
    if (index < 0) {
        index = 0;
    }
    if (index > static_cast<int>(well_->measurements.size())) {
        index = static_cast<int>(well_->measurements.size());
    }
    beginInsertRows(QModelIndex(), index, index);
    well_->measurements.insert(well_->measurements.begin() + index, point);
    well_->modified = true;
    endInsertRows();
    emit dataModified();
}

std::shared_ptr<WellData> MeasurementsModel::well() const {
    return well_;
}

bool MeasurementsModel::hasWell() const {
    return well_ != nullptr;
}

void MeasurementsModel::refresh() {
    beginResetModel();
    endResetModel();
}

}  // namespace incline3d::models

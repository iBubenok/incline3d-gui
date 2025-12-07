#include "models/results_model.h"

#include <QBrush>

namespace incline3d::models {

ResultsModel::ResultsModel(QObject* parent)
    : QAbstractTableModel(parent) {
}

int ResultsModel::rowCount(const QModelIndex& parent) const {
    if (parent.isValid() || !well_) {
        return 0;
    }
    return static_cast<int>(well_->results.size());
}

int ResultsModel::columnCount(const QModelIndex& parent) const {
    if (parent.isValid()) {
        return 0;
    }
    return kColumnCount;
}

QVariant ResultsModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid() || !well_ ||
        index.row() >= static_cast<int>(well_->results.size())) {
        return {};
    }

    const auto& point = well_->results[index.row()];

    if (role == Qt::DisplayRole) {
        switch (index.column()) {
            case kColumnDepth:
                return QString::number(point.measured_depth_m, 'f', 2);
            case kColumnInclination:
                return QString::number(point.inclination_deg, 'f', 2);
            case kColumnAzimuth:
                if (point.azimuth_deg.has_value()) {
                    return QString::number(point.azimuth_deg.value(), 'f', 2);
                }
                return QString("-");
            case kColumnAppliedAzimuth:
                return QString::number(point.applied_azimuth_deg, 'f', 2);
            case kColumnNorth:
                return QString::number(point.north_m, 'f', 2);
            case kColumnEast:
                return QString::number(point.east_m, 'f', 2);
            case kColumnTvd:
                return QString::number(point.tvd_m, 'f', 2);
            case kColumnDogleg:
                return QString::number(point.dogleg_angle_deg, 'f', 3);
            case kColumnIntensity10:
                return QString::number(point.intensity_10m, 'f', 2);
            case kColumnIntensityL:
                return QString::number(point.intensity_L, 'f', 2);
            case kColumnMistakeX:
                return QString::number(point.mistake_x, 'f', 3);
            case kColumnMistakeY:
                return QString::number(point.mistake_y, 'f', 3);
            case kColumnMistakeZ:
                return QString::number(point.mistake_z, 'f', 3);
            case kColumnMistakeAbsg:
                return QString::number(point.mistake_absg, 'f', 3);
        }
    }

    if (role == Qt::ToolTipRole) {
        switch (index.column()) {
            case kColumnDepth:
                return tr("Глубина по стволу, м");
            case kColumnInclination:
                return tr("Угол наклона от вертикали, °");
            case kColumnAzimuth:
                return tr("Исходный азимут, °");
            case kColumnAppliedAzimuth:
                return tr("Приведённый азимут (истинный), °");
            case kColumnNorth:
                return tr("Смещение на север, м");
            case kColumnEast:
                return tr("Смещение на восток, м");
            case kColumnTvd:
                return tr("Вертикальная глубина, м");
            case kColumnDogleg:
                return tr("Угол пространственного искривления, °");
            case kColumnIntensity10:
                return tr("Интенсивность на 10 м, °/10м");
            case kColumnIntensityL:
                return tr("Интенсивность на интервал L, °/L");
            case kColumnMistakeX:
                return tr("Ошибка по X, м");
            case kColumnMistakeY:
                return tr("Ошибка по Y, м");
            case kColumnMistakeZ:
                return tr("Ошибка по Z, м");
            case kColumnMistakeAbsg:
                return tr("Ошибка абсолютного смещения, м");
        }
    }

    if (role == Qt::TextAlignmentRole) {
        return static_cast<int>(Qt::AlignRight | Qt::AlignVCenter);
    }

    // Подсветка высокой интенсивности
    if (role == Qt::BackgroundRole) {
        if (index.column() == kColumnIntensity10 || index.column() == kColumnIntensityL) {
            double threshold = well_->params.intensity_threshold_deg;
            double value = (index.column() == kColumnIntensity10)
                               ? point.intensity_10m
                               : point.intensity_L;
            if (threshold > 0 && value > threshold) {
                return QBrush(QColor(255, 200, 200));  // Светло-красный
            }
        }
    }

    return {};
}

QVariant ResultsModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (orientation != Qt::Horizontal || role != Qt::DisplayRole) {
        return {};
    }

    switch (section) {
        case kColumnDepth:
            return tr("Глуб., м");
        case kColumnInclination:
            return tr("Угол, °");
        case kColumnAzimuth:
            return tr("Азим., °");
        case kColumnAppliedAzimuth:
            return tr("Прив.аз., °");
        case kColumnNorth:
            return tr("Север, м");
        case kColumnEast:
            return tr("Восток, м");
        case kColumnTvd:
            return tr("TVD, м");
        case kColumnDogleg:
            return tr("DL, °");
        case kColumnIntensity10:
            return tr("И10, °/10м");
        case kColumnIntensityL:
            return tr("ИL, °/L");
        case kColumnMistakeX:
            return tr("δX, м");
        case kColumnMistakeY:
            return tr("δY, м");
        case kColumnMistakeZ:
            return tr("δZ, м");
        case kColumnMistakeAbsg:
            return tr("δR, м");
    }

    return {};
}

Qt::ItemFlags ResultsModel::flags(const QModelIndex& index) const {
    if (!index.isValid()) {
        return Qt::NoItemFlags;
    }

    // Результаты только для чтения
    return Qt::ItemIsEnabled | Qt::ItemIsSelectable;
}

void ResultsModel::setWell(std::shared_ptr<WellData> well) {
    beginResetModel();
    well_ = std::move(well);
    endResetModel();
}

void ResultsModel::clearWell() {
    beginResetModel();
    well_.reset();
    endResetModel();
}

std::shared_ptr<WellData> ResultsModel::well() const {
    return well_;
}

bool ResultsModel::hasWell() const {
    return well_ != nullptr;
}

void ResultsModel::refresh() {
    beginResetModel();
    endResetModel();
}

}  // namespace incline3d::models

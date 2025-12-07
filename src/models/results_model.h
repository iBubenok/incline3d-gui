#pragma once

#include <QAbstractTableModel>
#include <memory>

#include "models/well_data.h"

namespace incline3d::models {

/// Qt-модель для таблицы результатов расчёта (РЕЗ_ОБР_ИНКЛ)
class ResultsModel : public QAbstractTableModel {
    Q_OBJECT

public:
    enum Column {
        kColumnDepth = 0,
        kColumnInclination,
        kColumnAzimuth,
        kColumnAppliedAzimuth,
        kColumnNorth,
        kColumnEast,
        kColumnTvd,
        kColumnDogleg,
        kColumnIntensity10,
        kColumnIntensityL,
        kColumnMistakeX,
        kColumnMistakeY,
        kColumnMistakeZ,
        kColumnMistakeAbsg,
        kColumnCount
    };

    explicit ResultsModel(QObject* parent = nullptr);

    // QAbstractTableModel interface
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;

    // Управление данными
    void setWell(std::shared_ptr<WellData> well);
    void clearWell();

    // Доступ к данным
    std::shared_ptr<WellData> well() const;
    bool hasWell() const;

    // Обновление после пересчёта
    void refresh();

private:
    std::shared_ptr<WellData> well_;
};

}  // namespace incline3d::models

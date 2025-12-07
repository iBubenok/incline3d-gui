#pragma once

#include <QAbstractTableModel>
#include <memory>

#include "models/well_data.h"

namespace incline3d::models {

/// Qt-модель для таблицы исходных замеров (ИНТЕРВАЛЫ_ИНКЛ / ЗНАЧЕНИЯ)
class MeasurementsModel : public QAbstractTableModel {
    Q_OBJECT

public:
    enum Column {
        kColumnDepth = 0,
        kColumnInclination,
        kColumnAzimuth,
        kColumnAzimuthType,
        kColumnCount
    };

    explicit MeasurementsModel(QObject* parent = nullptr);

    // QAbstractTableModel interface
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation,
                        int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    bool setData(const QModelIndex& index, const QVariant& value,
                 int role = Qt::EditRole) override;

    // Управление данными
    void setWell(std::shared_ptr<WellData> well);
    void clearWell();

    // Добавление/удаление точек
    void addPoint(const MeasuredPoint& point);
    void removePoint(int index);
    void insertPoint(int index, const MeasuredPoint& point);

    /// Обновить представление модели
    void refresh();

    // Доступ к данным
    std::shared_ptr<WellData> well() const;
    bool hasWell() const;

signals:
    void dataModified();

private:
    std::shared_ptr<WellData> well_;
};

}  // namespace incline3d::models

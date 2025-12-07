#pragma once

#include <QAbstractTableModel>
#include <vector>

#include "models/shot_point.h"

namespace incline3d::models {

/// Qt-модель для таблицы пунктов возбуждения
class ShotPointsModel : public QAbstractTableModel {
    Q_OBJECT

public:
    enum Column {
        kColumnVisible = 0,
        kColumnName,
        kColumnX,
        kColumnY,
        kColumnZ,
        kColumnMarker,
        kColumnColor,
        kColumnCount
    };

    explicit ShotPointsModel(QObject* parent = nullptr);

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
    void addPoint(const ShotPoint& point);
    void removePoint(int index);
    void clear();
    void setPoints(const std::vector<ShotPoint>& points);

    ShotPoint& pointAt(int index);
    const ShotPoint& pointAt(int index) const;
    int pointCount() const;
    std::vector<ShotPoint>& points();
    const std::vector<ShotPoint>& points() const;

signals:
    void pointVisibilityChanged(int index, bool visible);
    void pointDataChanged(int index);

private:
    std::vector<ShotPoint> points_;
};

}  // namespace incline3d::models

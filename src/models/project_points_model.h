#pragma once

#include <QAbstractTableModel>
#include <vector>

#include "models/project_point.h"

namespace incline3d::models {

/// Qt-модель для таблицы проектных точек
class ProjectPointsModel : public QAbstractTableModel {
    Q_OBJECT

public:
    enum Column {
        kColumnVisible = 0,
        kColumnName,
        kColumnAzimuth,
        kColumnShift,
        kColumnDepth,
        kColumnAbsDepth,
        kColumnRadius,
        kColumnFactOffset,
        kColumnColor,
        kColumnCount
    };

    explicit ProjectPointsModel(QObject* parent = nullptr);

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
    void addPoint(const ProjectPoint& point);
    void removePoint(int index);
    void clear();
    void setPoints(const std::vector<ProjectPoint>& points);

    ProjectPoint& pointAt(int index);
    const ProjectPoint& pointAt(int index) const;
    int pointCount() const;
    std::vector<ProjectPoint>& points();
    const std::vector<ProjectPoint>& points() const;

    // Обновление фактических значений точки
    void updateFactValues(int index);

signals:
    void pointVisibilityChanged(int index, bool visible);
    void pointDataChanged(int index);

private:
    std::vector<ProjectPoint> points_;
};

}  // namespace incline3d::models

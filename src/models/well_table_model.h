#pragma once

#include <QAbstractTableModel>
#include <memory>
#include <vector>

#include "models/well_data.h"

namespace incline3d::models {

/// Qt-модель для таблицы скважин
class WellTableModel : public QAbstractTableModel {
    Q_OBJECT

public:
    enum Column {
        kColumnVisible = 0,
        kColumnName,
        kColumnField,
        kColumnCluster,
        kColumnDepth,
        kColumnMaxAngle,
        kColumnMaxIntensity,
        kColumnDisplacement,
        kColumnColor,
        kColumnCount
    };

    explicit WellTableModel(QObject* parent = nullptr);

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
    void addWell(std::shared_ptr<WellData> well);
    void removeWell(int index);
    void clear();

    std::shared_ptr<WellData> wellAt(int index) const;
    int wellCount() const;
    std::vector<std::shared_ptr<WellData>>& wells();
    const std::vector<std::shared_ptr<WellData>>& wells() const;

    // Поиск скважины по имени
    int findWellByName(const std::string& name) const;

    // Обновление данных скважины (вызывает dataChanged)
    void updateWell(int index);

signals:
    void wellVisibilityChanged(int index, bool visible);
    void wellColorChanged(int index, const QColor& color);
    void wellDataChanged(int index);

private:
    std::vector<std::shared_ptr<WellData>> wells_;
};

}  // namespace incline3d::models

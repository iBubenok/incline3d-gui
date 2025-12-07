#pragma once

#include <QDockWidget>
#include <QTableView>

namespace incline3d::models {
class WellTableModel;
}

namespace incline3d::ui {

class WellsDock : public QDockWidget {
    Q_OBJECT

public:
    explicit WellsDock(models::WellTableModel* model, QWidget* parent = nullptr);

signals:
    void wellSelected(int index);

private slots:
    void onSelectionChanged();
    void onDoubleClicked(const QModelIndex& index);

private:
    QTableView* table_view_{nullptr};
    models::WellTableModel* model_{nullptr};
};

}  // namespace incline3d::ui

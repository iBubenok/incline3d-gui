#pragma once
#include <QDockWidget>
#include <QTableView>

namespace incline3d::models { class MeasurementsModel; }

namespace incline3d::ui {

class MeasurementsDock : public QDockWidget {
    Q_OBJECT
public:
    explicit MeasurementsDock(models::MeasurementsModel* model, QWidget* parent = nullptr);
private:
    QTableView* table_view_{nullptr};
};

}  // namespace incline3d::ui

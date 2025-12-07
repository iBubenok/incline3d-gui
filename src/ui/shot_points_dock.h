#pragma once
#include <QDockWidget>
#include <QTableView>

namespace incline3d::models { class ShotPointsModel; }

namespace incline3d::ui {

class ShotPointsDock : public QDockWidget {
    Q_OBJECT
public:
    explicit ShotPointsDock(models::ShotPointsModel* model, QWidget* parent = nullptr);
private:
    QTableView* table_view_{nullptr};
};

}  // namespace incline3d::ui

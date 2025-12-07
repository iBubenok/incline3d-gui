#pragma once
#include <QDockWidget>
#include <QTableView>

namespace incline3d::models { class ProjectPointsModel; }

namespace incline3d::ui {

class ProjectPointsDock : public QDockWidget {
    Q_OBJECT
public:
    explicit ProjectPointsDock(models::ProjectPointsModel* model, QWidget* parent = nullptr);
private:
    QTableView* table_view_{nullptr};
};

}  // namespace incline3d::ui

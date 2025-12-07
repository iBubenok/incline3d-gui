#pragma once
#include <QDockWidget>
#include <QTableView>

namespace incline3d::models { class ResultsModel; }

namespace incline3d::ui {

class ResultsDock : public QDockWidget {
    Q_OBJECT
public:
    explicit ResultsDock(models::ResultsModel* model, QWidget* parent = nullptr);
private:
    QTableView* table_view_{nullptr};
};

}  // namespace incline3d::ui

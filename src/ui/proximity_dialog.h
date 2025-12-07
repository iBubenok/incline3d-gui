#pragma once
#include <QDialog>

class QComboBox;
class QDoubleSpinBox;
class QLabel;

namespace incline3d::models { class WellTableModel; }
namespace incline3d::core { class InclineProcessRunner; }

namespace incline3d::ui {

class ProximityDialog : public QDialog {
    Q_OBJECT
public:
    ProximityDialog(models::WellTableModel* model,
                    core::InclineProcessRunner* runner,
                    QWidget* parent = nullptr);

private slots:
    void onCalculate();

private:
    models::WellTableModel* model_;
    core::InclineProcessRunner* runner_;
    QComboBox* well_a_combo_{nullptr};
    QComboBox* well_b_combo_{nullptr};
    QDoubleSpinBox* tolerance_spin_{nullptr};
    QLabel* result_label_{nullptr};
};

}  // namespace incline3d::ui

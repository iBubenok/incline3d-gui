#pragma once
#include <QDialog>

namespace incline3d::ui {

class AboutDialog : public QDialog {
    Q_OBJECT
public:
    explicit AboutDialog(QWidget* parent = nullptr);
};

}  // namespace incline3d::ui

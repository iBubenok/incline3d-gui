#pragma once
#include <QDialog>

class QLineEdit;
class QSpinBox;
class QCheckBox;

namespace incline3d::ui {

class SettingsDialog : public QDialog {
    Q_OBJECT
public:
    explicit SettingsDialog(QWidget* parent = nullptr);

private slots:
    void onBrowseInclproc();
    void onAccept();

private:
    void setupUi();
    void loadSettings();

    QLineEdit* inclproc_path_edit_{nullptr};
    QSpinBox* autosave_interval_spin_{nullptr};
    QCheckBox* autosave_enabled_check_{nullptr};
};

}  // namespace incline3d::ui

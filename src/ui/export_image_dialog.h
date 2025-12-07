#pragma once

#include <QDialog>
#include <QImage>

class QSpinBox;
class QComboBox;
class QCheckBox;
class QLineEdit;
class QPushButton;
class QLabel;

namespace incline3d::ui {

/// Диалог экспорта изображения вида
class ExportImageDialog : public QDialog {
    Q_OBJECT

public:
    enum class ExportSource {
        kView3D,
        kPlanView,
        kVerticalView,
        kCurrentView
    };

    explicit ExportImageDialog(QWidget* parent = nullptr);
    ~ExportImageDialog() override = default;

    /// Установить источник изображения
    void setSource(ExportSource source);

    /// Установить виджет для захвата
    void setCaptureWidget(QWidget* widget);

    /// Получить результирующее изображение
    QImage resultImage() const { return result_image_; }

    /// Получить выбранный путь
    QString selectedPath() const;

    /// Был ли выполнен экспорт в файл
    bool exportedToFile() const { return exported_to_file_; }

    /// Было ли выполнено копирование в буфер обмена
    bool copiedToClipboard() const { return copied_to_clipboard_; }

private slots:
    void onBrowse();
    void onExportToFile();
    void onCopyToClipboard();
    void updatePreview();

private:
    void setupUi();
    QImage captureView();

    QWidget* capture_widget_{nullptr};

    // Элементы управления
    QSpinBox* width_spin_{nullptr};
    QSpinBox* height_spin_{nullptr};
    QCheckBox* keep_ratio_check_{nullptr};
    QComboBox* format_combo_{nullptr};
    QSpinBox* quality_spin_{nullptr};
    QLineEdit* path_edit_{nullptr};
    QPushButton* browse_button_{nullptr};
    QPushButton* export_button_{nullptr};
    QPushButton* clipboard_button_{nullptr};
    QLabel* preview_label_{nullptr};

    QImage result_image_;
    bool exported_to_file_{false};
    bool copied_to_clipboard_{false};

    double aspect_ratio_{1.0};
    bool updating_size_{false};
};

}  // namespace incline3d::ui

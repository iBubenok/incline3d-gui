#include "ui/export_image_dialog.h"

#include <QApplication>
#include <QCheckBox>
#include <QClipboard>
#include <QComboBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QVBoxLayout>

#include "utils/logger.h"

namespace incline3d::ui {

ExportImageDialog::ExportImageDialog(QWidget* parent)
    : QDialog(parent) {
    setWindowTitle(tr("Экспорт изображения"));
    setMinimumSize(500, 400);
    setupUi();
}

void ExportImageDialog::setupUi() {
    auto* main_layout = new QVBoxLayout(this);

    // Группа размера
    auto* size_group = new QGroupBox(tr("Размер изображения"), this);
    auto* size_layout = new QGridLayout(size_group);

    size_layout->addWidget(new QLabel(tr("Ширина:"), this), 0, 0);
    width_spin_ = new QSpinBox(this);
    width_spin_->setRange(100, 10000);
    width_spin_->setValue(1920);
    width_spin_->setSuffix(tr(" пикс."));
    size_layout->addWidget(width_spin_, 0, 1);

    size_layout->addWidget(new QLabel(tr("Высота:"), this), 1, 0);
    height_spin_ = new QSpinBox(this);
    height_spin_->setRange(100, 10000);
    height_spin_->setValue(1080);
    height_spin_->setSuffix(tr(" пикс."));
    size_layout->addWidget(height_spin_, 1, 1);

    keep_ratio_check_ = new QCheckBox(tr("Сохранять пропорции"), this);
    keep_ratio_check_->setChecked(true);
    size_layout->addWidget(keep_ratio_check_, 2, 0, 1, 2);

    main_layout->addWidget(size_group);

    // Группа формата
    auto* format_group = new QGroupBox(tr("Формат"), this);
    auto* format_layout = new QGridLayout(format_group);

    format_layout->addWidget(new QLabel(tr("Формат файла:"), this), 0, 0);
    format_combo_ = new QComboBox(this);
    format_combo_->addItem("PNG (*.png)", "png");
    format_combo_->addItem("JPEG (*.jpg)", "jpg");
    format_combo_->addItem("BMP (*.bmp)", "bmp");
    format_combo_->addItem("TIFF (*.tiff)", "tiff");
    format_layout->addWidget(format_combo_, 0, 1);

    format_layout->addWidget(new QLabel(tr("Качество JPEG:"), this), 1, 0);
    quality_spin_ = new QSpinBox(this);
    quality_spin_->setRange(1, 100);
    quality_spin_->setValue(95);
    quality_spin_->setSuffix("%");
    format_layout->addWidget(quality_spin_, 1, 1);

    main_layout->addWidget(format_group);

    // Путь к файлу
    auto* path_group = new QGroupBox(tr("Путь к файлу"), this);
    auto* path_layout = new QHBoxLayout(path_group);

    path_edit_ = new QLineEdit(this);
    path_edit_->setPlaceholderText(tr("Укажите путь для сохранения..."));
    path_layout->addWidget(path_edit_);

    browse_button_ = new QPushButton(tr("Обзор..."), this);
    path_layout->addWidget(browse_button_);

    main_layout->addWidget(path_group);

    // Предпросмотр
    auto* preview_group = new QGroupBox(tr("Предпросмотр"), this);
    auto* preview_layout = new QVBoxLayout(preview_group);

    preview_label_ = new QLabel(this);
    preview_label_->setMinimumSize(200, 150);
    preview_label_->setAlignment(Qt::AlignCenter);
    preview_label_->setStyleSheet("QLabel { background-color: #f0f0f0; border: 1px solid #ccc; }");
    preview_layout->addWidget(preview_label_);

    main_layout->addWidget(preview_group);

    // Кнопки
    auto* button_layout = new QHBoxLayout();

    clipboard_button_ = new QPushButton(tr("Копировать в буфер"), this);
    button_layout->addWidget(clipboard_button_);

    export_button_ = new QPushButton(tr("Экспорт в файл"), this);
    export_button_->setDefault(true);
    button_layout->addWidget(export_button_);

    auto* cancel_button = new QPushButton(tr("Отмена"), this);
    button_layout->addWidget(cancel_button);

    main_layout->addLayout(button_layout);

    // Подключение сигналов
    connect(width_spin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int value) {
                if (keep_ratio_check_->isChecked() && !updating_size_) {
                    updating_size_ = true;
                    height_spin_->setValue(static_cast<int>(value / aspect_ratio_));
                    updating_size_ = false;
                }
            });

    connect(height_spin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, [this](int value) {
                if (keep_ratio_check_->isChecked() && !updating_size_) {
                    updating_size_ = true;
                    width_spin_->setValue(static_cast<int>(value * aspect_ratio_));
                    updating_size_ = false;
                }
            });

    connect(browse_button_, &QPushButton::clicked, this, &ExportImageDialog::onBrowse);
    connect(export_button_, &QPushButton::clicked, this, &ExportImageDialog::onExportToFile);
    connect(clipboard_button_, &QPushButton::clicked, this, &ExportImageDialog::onCopyToClipboard);
    connect(cancel_button, &QPushButton::clicked, this, &QDialog::reject);

    connect(format_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, [this](int index) {
                quality_spin_->setEnabled(format_combo_->currentData().toString() == "jpg");
            });

    quality_spin_->setEnabled(false);
}

void ExportImageDialog::setSource(ExportSource source) {
    Q_UNUSED(source)
}

void ExportImageDialog::setCaptureWidget(QWidget* widget) {
    capture_widget_ = widget;

    if (widget) {
        // Установка размеров по размеру виджета
        width_spin_->setValue(widget->width());
        height_spin_->setValue(widget->height());
        aspect_ratio_ = static_cast<double>(widget->width()) / widget->height();

        // Обновление предпросмотра
        updatePreview();
    }
}

QString ExportImageDialog::selectedPath() const {
    return path_edit_->text();
}

void ExportImageDialog::onBrowse() {
    QString format = format_combo_->currentData().toString();
    QString filter;

    if (format == "png") filter = tr("PNG файлы (*.png)");
    else if (format == "jpg") filter = tr("JPEG файлы (*.jpg *.jpeg)");
    else if (format == "bmp") filter = tr("BMP файлы (*.bmp)");
    else if (format == "tiff") filter = tr("TIFF файлы (*.tiff *.tif)");

    QString path = QFileDialog::getSaveFileName(
        this, tr("Сохранить изображение"),
        path_edit_->text(), filter);

    if (!path.isEmpty()) {
        // Добавление расширения если нужно
        if (!path.endsWith("." + format, Qt::CaseInsensitive)) {
            path += "." + format;
        }
        path_edit_->setText(path);
    }
}

void ExportImageDialog::onExportToFile() {
    if (path_edit_->text().isEmpty()) {
        QMessageBox::warning(this, tr("Экспорт"),
                             tr("Укажите путь для сохранения файла"));
        return;
    }

    result_image_ = captureView();

    if (result_image_.isNull()) {
        QMessageBox::critical(this, tr("Ошибка"),
                              tr("Не удалось создать изображение"));
        return;
    }

    // Масштабирование если нужно
    if (result_image_.width() != width_spin_->value() ||
        result_image_.height() != height_spin_->value()) {
        result_image_ = result_image_.scaled(
            width_spin_->value(), height_spin_->value(),
            Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    QString format = format_combo_->currentData().toString();
    int quality = (format == "jpg") ? quality_spin_->value() : -1;

    if (result_image_.save(path_edit_->text(), format.toUpper().toLatin1().constData(), quality)) {
        exported_to_file_ = true;
        LOG_INFO(tr("Изображение сохранено: %1").arg(path_edit_->text()));
        accept();
    } else {
        QMessageBox::critical(this, tr("Ошибка"),
                              tr("Не удалось сохранить изображение"));
    }
}

void ExportImageDialog::onCopyToClipboard() {
    result_image_ = captureView();

    if (result_image_.isNull()) {
        QMessageBox::critical(this, tr("Ошибка"),
                              tr("Не удалось создать изображение"));
        return;
    }

    // Масштабирование если нужно
    if (result_image_.width() != width_spin_->value() ||
        result_image_.height() != height_spin_->value()) {
        result_image_ = result_image_.scaled(
            width_spin_->value(), height_spin_->value(),
            Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
    }

    QApplication::clipboard()->setImage(result_image_);
    copied_to_clipboard_ = true;

    LOG_INFO(tr("Изображение скопировано в буфер обмена"));

    QMessageBox::information(this, tr("Экспорт"),
                             tr("Изображение скопировано в буфер обмена"));
}

void ExportImageDialog::updatePreview() {
    QImage preview = captureView();

    if (!preview.isNull()) {
        // Масштабирование для предпросмотра
        preview = preview.scaled(preview_label_->size() - QSize(10, 10),
                                 Qt::KeepAspectRatio, Qt::SmoothTransformation);
        preview_label_->setPixmap(QPixmap::fromImage(preview));
    }
}

QImage ExportImageDialog::captureView() {
    if (!capture_widget_) {
        return QImage();
    }

    // Захват содержимого виджета
    return capture_widget_->grab().toImage();
}

}  // namespace incline3d::ui

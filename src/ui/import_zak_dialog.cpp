#include "ui/import_zak_dialog.h"

#include <QBoxLayout>
#include <QCheckBox>
#include <QComboBox>
#include <QDialogButtonBox>
#include <QFile>
#include <QFileDialog>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QSpinBox>
#include <QTableWidget>
#include <QTextEdit>
#include <QTextStream>

#include "utils/angle_utils.h"
#include "utils/logger.h"

namespace incline3d::ui {

ImportZakDialog::ImportZakDialog(const QString& file_path, QWidget* parent)
    : QDialog(parent)
    , file_path_(file_path) {
    well_ = std::make_shared<models::WellData>();
    setupUi();

    if (!file_path_.isEmpty()) {
        file_path_edit_->setText(file_path_);
        onLoadFile();
    }
}

ImportZakDialog::~ImportZakDialog() = default;

void ImportZakDialog::setupUi() {
    setWindowTitle(tr("Импорт из текстового файла (ЗАК)"));
    setMinimumSize(800, 600);
    resize(900, 700);

    auto* main_layout = new QVBoxLayout(this);

    // Группа выбора файла
    auto* file_group = new QGroupBox(tr("Файл"), this);
    auto* file_layout = new QHBoxLayout(file_group);

    file_path_edit_ = new QLineEdit(file_group);
    file_path_edit_->setReadOnly(true);
    file_layout->addWidget(file_path_edit_);

    browse_btn_ = new QPushButton(tr("Обзор..."), file_group);
    connect(browse_btn_, &QPushButton::clicked, this, [this]() {
        QString path = QFileDialog::getOpenFileName(
            this, tr("Открыть текстовый файл"),
            QString(),
            tr("Текстовые файлы (*.txt *.zak *.csv);;Все файлы (*)"));
        if (!path.isEmpty()) {
            file_path_ = path;
            file_path_edit_->setText(path);
            onLoadFile();
        }
    });
    file_layout->addWidget(browse_btn_);

    main_layout->addWidget(file_group);

    // Группа настроек парсера
    auto* settings_group = new QGroupBox(tr("Настройки парсера"), this);
    auto* settings_layout = new QFormLayout(settings_group);

    separator_combo_ = new QComboBox(settings_group);
    separator_combo_->addItem(tr("Точка с запятой (;)"), ";");
    separator_combo_->addItem(tr("Табуляция"), "\t");
    separator_combo_->addItem(tr("Запятая (,)"), ",");
    separator_combo_->addItem(tr("Пробелы"), " ");
    connect(separator_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ImportZakDialog::onSettingsChanged);
    settings_layout->addRow(tr("Разделитель:"), separator_combo_);

    decimal_separator_combo_ = new QComboBox(settings_group);
    decimal_separator_combo_->addItem(tr("Точка (.)"), ".");
    decimal_separator_combo_->addItem(tr("Запятая (,)"), ",");
    connect(decimal_separator_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ImportZakDialog::onSettingsChanged);
    settings_layout->addRow(tr("Десятичный разделитель:"), decimal_separator_combo_);

    skip_lines_spin_ = new QSpinBox(settings_group);
    skip_lines_spin_->setRange(0, 100);
    skip_lines_spin_->setValue(0);
    skip_lines_spin_->setToolTip(tr("Количество строк заголовка для пропуска"));
    connect(skip_lines_spin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ImportZakDialog::onSettingsChanged);
    settings_layout->addRow(tr("Пропустить строк:"), skip_lines_spin_);

    auto* columns_layout = new QHBoxLayout();

    depth_col_spin_ = new QSpinBox(settings_group);
    depth_col_spin_->setRange(1, 20);
    depth_col_spin_->setValue(1);
    connect(depth_col_spin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ImportZakDialog::onSettingsChanged);
    columns_layout->addWidget(new QLabel(tr("Глубина:"), settings_group));
    columns_layout->addWidget(depth_col_spin_);

    angle_col_spin_ = new QSpinBox(settings_group);
    angle_col_spin_->setRange(1, 20);
    angle_col_spin_->setValue(2);
    connect(angle_col_spin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ImportZakDialog::onSettingsChanged);
    columns_layout->addWidget(new QLabel(tr("Угол:"), settings_group));
    columns_layout->addWidget(angle_col_spin_);

    azimuth_col_spin_ = new QSpinBox(settings_group);
    azimuth_col_spin_->setRange(0, 20);
    azimuth_col_spin_->setValue(3);
    azimuth_col_spin_->setSpecialValueText(tr("Нет"));
    connect(azimuth_col_spin_, QOverload<int>::of(&QSpinBox::valueChanged),
            this, &ImportZakDialog::onSettingsChanged);
    columns_layout->addWidget(new QLabel(tr("Азимут:"), settings_group));
    columns_layout->addWidget(azimuth_col_spin_);

    columns_layout->addStretch();
    settings_layout->addRow(tr("Колонки:"), columns_layout);

    auto* angle_format_layout = new QHBoxLayout();
    angle_degmin_check_ = new QCheckBox(tr("Угол в гр.мин"), settings_group);
    angle_degmin_check_->setToolTip(tr("Значения угла в формате градусы.минуты (45.30 = 45°30')"));
    connect(angle_degmin_check_, &QCheckBox::toggled, this, &ImportZakDialog::onSettingsChanged);
    angle_format_layout->addWidget(angle_degmin_check_);

    azimuth_degmin_check_ = new QCheckBox(tr("Азимут в гр.мин"), settings_group);
    azimuth_degmin_check_->setToolTip(tr("Значения азимута в формате градусы.минуты"));
    connect(azimuth_degmin_check_, &QCheckBox::toggled, this, &ImportZakDialog::onSettingsChanged);
    angle_format_layout->addWidget(azimuth_degmin_check_);
    angle_format_layout->addStretch();
    settings_layout->addRow(tr("Формат углов:"), angle_format_layout);

    main_layout->addWidget(settings_group);

    // Группа метаданных
    auto* meta_group = new QGroupBox(tr("Метаданные"), this);
    auto* meta_layout = new QFormLayout(meta_group);

    well_name_edit_ = new QLineEdit(meta_group);
    meta_layout->addRow(tr("Название скважины:"), well_name_edit_);

    main_layout->addWidget(meta_group);

    // Предпросмотр данных
    auto* preview_group = new QGroupBox(tr("Предпросмотр данных"), this);
    auto* preview_layout = new QVBoxLayout(preview_group);

    preview_table_ = new QTableWidget(preview_group);
    preview_table_->setColumnCount(4);
    preview_table_->setHorizontalHeaderLabels({tr("Глубина"), tr("Угол"), tr("Азимут"), tr("Статус")});
    preview_table_->horizontalHeader()->setStretchLastSection(true);
    preview_table_->setAlternatingRowColors(true);
    preview_table_->setEditTriggers(QAbstractItemView::NoEditTriggers);
    preview_layout->addWidget(preview_table_);

    points_count_label_ = new QLabel(tr("Точек: 0"), preview_group);
    preview_layout->addWidget(points_count_label_);

    main_layout->addWidget(preview_group, 1);

    // Лог
    auto* log_group = new QGroupBox(tr("Сообщения"), this);
    auto* log_layout = new QVBoxLayout(log_group);
    log_text_ = new QTextEdit(log_group);
    log_text_->setReadOnly(true);
    log_text_->setMaximumHeight(80);
    log_layout->addWidget(log_text_);
    main_layout->addWidget(log_group);

    // Кнопки
    auto* button_box = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    button_box->button(QDialogButtonBox::Ok)->setText(tr("Импортировать"));
    connect(button_box, &QDialogButtonBox::accepted, this, &ImportZakDialog::onImport);
    connect(button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    main_layout->addWidget(button_box);
}

void ImportZakDialog::onLoadFile() {
    parseFile();
    updatePreview();
}

void ImportZakDialog::parseFile() {
    file_lines_.clear();
    log_text_->clear();

    QFile file(file_path_);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        log_text_->append(tr("Ошибка: не удалось открыть файл"));
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        file_lines_.append(in.readLine());
    }
    file.close();

    log_text_->append(tr("Загружено строк: %1").arg(file_lines_.size()));

    // Попытка автоопределения названия скважины из имени файла
    if (well_name_edit_->text().isEmpty()) {
        QFileInfo fi(file_path_);
        well_name_edit_->setText(fi.baseName());
    }
}

void ImportZakDialog::onSettingsChanged() {
    updatePreview();
}

void ImportZakDialog::updatePreview() {
    preview_table_->setRowCount(0);

    if (file_lines_.isEmpty()) {
        points_count_label_->setText(tr("Точек: 0"));
        return;
    }

    QString separator = separator_combo_->currentData().toString();
    QString decimal_sep = decimal_separator_combo_->currentData().toString();
    int skip_lines = skip_lines_spin_->value();
    int depth_col = depth_col_spin_->value() - 1;  // 0-based
    int angle_col = angle_col_spin_->value() - 1;
    int azimuth_col = azimuth_col_spin_->value() - 1;  // -1 если не выбрано
    bool angle_degmin = angle_degmin_check_->isChecked();
    bool azimuth_degmin = azimuth_degmin_check_->isChecked();

    int valid_count = 0;
    int max_preview = 100;
    QRegularExpression split_re;

    if (separator == " ") {
        split_re = QRegularExpression("\\s+");
    } else {
        split_re = QRegularExpression(QRegularExpression::escape(separator));
    }

    for (int i = skip_lines; i < file_lines_.size() && preview_table_->rowCount() < max_preview; ++i) {
        QString line = file_lines_[i].trimmed();
        if (line.isEmpty() || line.startsWith('#') || line.startsWith("//")) {
            continue;
        }

        QStringList parts = line.split(split_re, Qt::SkipEmptyParts);

        if (parts.size() <= depth_col || parts.size() <= angle_col) {
            continue;
        }

        // Получение значений с заменой десятичного разделителя
        QString depth_str = parts[depth_col];
        QString angle_str = parts[angle_col];
        QString azimuth_str = (azimuth_col >= 0 && azimuth_col < parts.size()) ? parts[azimuth_col] : QString();

        if (decimal_sep == ",") {
            depth_str.replace(',', '.');
            angle_str.replace(',', '.');
            azimuth_str.replace(',', '.');
        }

        bool ok_depth, ok_angle, ok_azimuth;
        double depth = depth_str.toDouble(&ok_depth);
        double angle = angle_str.toDouble(&ok_angle);
        double azimuth = azimuth_str.isEmpty() ? 0.0 : azimuth_str.toDouble(&ok_azimuth);
        bool has_azimuth = !azimuth_str.isEmpty() && ok_azimuth;

        if (!ok_depth || !ok_angle) {
            continue;
        }

        // Конвертация из градусы.минуты
        if (angle_degmin) {
            angle = utils::deg_from_degmin(angle);
        }
        if (azimuth_degmin && has_azimuth) {
            azimuth = utils::deg_from_degmin(azimuth);
        }

        int row = preview_table_->rowCount();
        preview_table_->insertRow(row);

        preview_table_->setItem(row, 0, new QTableWidgetItem(QString::number(depth, 'f', 2)));
        preview_table_->setItem(row, 1, new QTableWidgetItem(QString::number(angle, 'f', 2)));

        if (has_azimuth) {
            preview_table_->setItem(row, 2, new QTableWidgetItem(QString::number(azimuth, 'f', 2)));
        } else {
            preview_table_->setItem(row, 2, new QTableWidgetItem("-"));
        }

        QString status;
        if (angle < 0 || angle > 120) {
            status = tr("Ошибка: угол");
        } else if (has_azimuth && (azimuth < 0 || azimuth > 360)) {
            status = tr("Предупреждение: азимут");
        } else {
            status = tr("OK");
        }
        preview_table_->setItem(row, 3, new QTableWidgetItem(status));

        ++valid_count;
    }

    points_count_label_->setText(tr("Точек: %1").arg(valid_count));
}

void ImportZakDialog::onImport() {
    if (file_lines_.isEmpty()) {
        QMessageBox::warning(this, tr("Импорт"),
                             tr("Нет данных для импорта. Загрузите файл."));
        return;
    }

    // Заполнение метаданных
    well_->metadata.well_name = well_name_edit_->text().toStdString();
    well_->metadata.file_name = file_path_.toStdString();
    well_->source_file_path = file_path_.toStdString();
    well_->source_format = "zak";

    // Очистка предыдущих измерений
    well_->measurements.clear();

    QString separator = separator_combo_->currentData().toString();
    QString decimal_sep = decimal_separator_combo_->currentData().toString();
    int skip_lines = skip_lines_spin_->value();
    int depth_col = depth_col_spin_->value() - 1;
    int angle_col = angle_col_spin_->value() - 1;
    int azimuth_col = azimuth_col_spin_->value() - 1;
    bool angle_degmin = angle_degmin_check_->isChecked();
    bool azimuth_degmin = azimuth_degmin_check_->isChecked();

    QRegularExpression split_re;
    if (separator == " ") {
        split_re = QRegularExpression("\\s+");
    } else {
        split_re = QRegularExpression(QRegularExpression::escape(separator));
    }

    int imported = 0;
    int skipped = 0;

    for (int i = skip_lines; i < file_lines_.size(); ++i) {
        QString line = file_lines_[i].trimmed();
        if (line.isEmpty() || line.startsWith('#') || line.startsWith("//")) {
            continue;
        }

        QStringList parts = line.split(split_re, Qt::SkipEmptyParts);

        if (parts.size() <= depth_col || parts.size() <= angle_col) {
            ++skipped;
            continue;
        }

        QString depth_str = parts[depth_col];
        QString angle_str = parts[angle_col];
        QString azimuth_str = (azimuth_col >= 0 && azimuth_col < parts.size()) ? parts[azimuth_col] : QString();

        if (decimal_sep == ",") {
            depth_str.replace(',', '.');
            angle_str.replace(',', '.');
            azimuth_str.replace(',', '.');
        }

        bool ok_depth, ok_angle, ok_azimuth;
        double depth = depth_str.toDouble(&ok_depth);
        double angle = angle_str.toDouble(&ok_angle);
        double azimuth = azimuth_str.isEmpty() ? 0.0 : azimuth_str.toDouble(&ok_azimuth);
        bool has_azimuth = !azimuth_str.isEmpty() && ok_azimuth;

        if (!ok_depth || !ok_angle) {
            ++skipped;
            continue;
        }

        if (angle_degmin) {
            angle = utils::deg_from_degmin(angle);
        }
        if (azimuth_degmin && has_azimuth) {
            azimuth = utils::deg_from_degmin(azimuth);
        }

        models::MeasuredPoint point;
        point.measured_depth_m = depth;
        point.inclination_deg = angle;
        if (has_azimuth) {
            point.azimuth_deg = azimuth;
            point.azimuth_type = models::AzimuthType::kMagnetic;
        }

        well_->measurements.push_back(point);
        ++imported;
    }

    if (imported == 0) {
        QMessageBox::warning(this, tr("Импорт"),
                             tr("Не удалось импортировать данные. Проверьте настройки парсера."));
        return;
    }

    import_successful_ = true;
    LOG_INFO(tr("Импортировано из ЗАК: %1 точек, пропущено: %2").arg(imported).arg(skipped));

    QMessageBox::information(this, tr("Импорт"),
                             tr("Импортировано точек: %1\nПропущено: %2").arg(imported).arg(skipped));

    accept();
}

std::shared_ptr<models::WellData> ImportZakDialog::wellData() const {
    return well_;
}

}  // namespace incline3d::ui

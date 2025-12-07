#include "ui/import_las_dialog.h"

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
#include <QTableWidget>
#include <QTextEdit>
#include <QTextStream>
#include <QRegularExpression>

#include "utils/angle_utils.h"
#include "utils/logger.h"

namespace incline3d::ui {

ImportLasDialog::ImportLasDialog(const QString& file_path, QWidget* parent)
    : QDialog(parent)
    , file_path_(file_path) {
    well_ = std::make_shared<models::WellData>();
    setupUi();

    if (!file_path_.isEmpty()) {
        file_path_edit_->setText(file_path_);
        onLoadFile();
    }
}

ImportLasDialog::~ImportLasDialog() = default;

void ImportLasDialog::setupUi() {
    setWindowTitle(tr("Импорт из LAS-файла"));
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
            this, tr("Открыть LAS-файл"),
            QString(),
            tr("LAS файлы (*.las *.LAS);;Все файлы (*)"));
        if (!path.isEmpty()) {
            file_path_ = path;
            file_path_edit_->setText(path);
            onLoadFile();
        }
    });
    file_layout->addWidget(browse_btn_);

    main_layout->addWidget(file_group);

    // Группа информации о скважине
    auto* info_group = new QGroupBox(tr("Информация из файла"), this);
    auto* info_layout = new QFormLayout(info_group);

    well_name_edit_ = new QLineEdit(info_group);
    info_layout->addRow(tr("Скважина:"), well_name_edit_);

    field_edit_ = new QLineEdit(info_group);
    info_layout->addRow(tr("Месторождение:"), field_edit_);

    uwi_edit_ = new QLineEdit(info_group);
    info_layout->addRow(tr("UWI:"), uwi_edit_);

    main_layout->addWidget(info_group);

    // Группа выбора кривых
    auto* curves_group = new QGroupBox(tr("Соответствие кривых"), this);
    auto* curves_layout = new QFormLayout(curves_group);

    depth_curve_combo_ = new QComboBox(curves_group);
    curves_layout->addRow(tr("Глубина (DEPT/MD):"), depth_curve_combo_);

    auto* angle_layout = new QHBoxLayout();
    angle_curve_combo_ = new QComboBox(curves_group);
    angle_layout->addWidget(angle_curve_combo_);
    angle_degmin_check_ = new QCheckBox(tr("Гр.мин"), curves_group);
    angle_degmin_check_->setToolTip(tr("Значения в формате градусы.минуты (45.30 = 45°30')"));
    angle_layout->addWidget(angle_degmin_check_);
    curves_layout->addRow(tr("Угол (INCL/INC):"), angle_layout);

    auto* azimuth_layout = new QHBoxLayout();
    azimuth_curve_combo_ = new QComboBox(curves_group);
    azimuth_layout->addWidget(azimuth_curve_combo_);
    azimuth_degmin_check_ = new QCheckBox(tr("Гр.мин"), curves_group);
    azimuth_degmin_check_->setToolTip(tr("Значения в формате градусы.минуты (45.30 = 45°30')"));
    azimuth_layout->addWidget(azimuth_degmin_check_);
    curves_layout->addRow(tr("Азимут (AZIM/AZ):"), azimuth_layout);

    connect(depth_curve_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ImportLasDialog::onCurveSelectionChanged);
    connect(angle_curve_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ImportLasDialog::onCurveSelectionChanged);
    connect(azimuth_curve_combo_, QOverload<int>::of(&QComboBox::currentIndexChanged),
            this, &ImportLasDialog::onCurveSelectionChanged);

    main_layout->addWidget(curves_group);

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
    log_text_->setMaximumHeight(100);
    log_layout->addWidget(log_text_);
    main_layout->addWidget(log_group);

    // Кнопки
    auto* button_box = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    button_box->button(QDialogButtonBox::Ok)->setText(tr("Импортировать"));
    connect(button_box, &QDialogButtonBox::accepted, this, &ImportLasDialog::onImport);
    connect(button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);
    main_layout->addWidget(button_box);
}

void ImportLasDialog::onLoadFile() {
    parseLasFile();
    populateCurveComboBoxes();
    updatePreview();
}

void ImportLasDialog::parseLasFile() {
    las_data_ = LasData();
    log_text_->clear();

    QFile file(file_path_);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        log_text_->append(tr("Ошибка: не удалось открыть файл"));
        return;
    }

    QTextStream in(&file);
    QString current_section;
    int curve_count = 0;
    bool in_data_section = false;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        // Пропуск комментариев и пустых строк
        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }

        // Определение секции
        if (line.startsWith('~')) {
            QString section = line.mid(1, 1).toUpper();
            current_section = section;
            in_data_section = (section == "A");  // ~A = ASCII data section
            continue;
        }

        // Парсинг секций
        if (current_section == "W") {
            // Well Information Section
            QRegularExpression re(R"(^(\w+)\s*\.\s*[^:]*:\s*(.*)$)");
            auto match = re.match(line);
            if (match.hasMatch()) {
                QString mnemonic = match.captured(1).toUpper();
                QString value = match.captured(2).trimmed();

                if (mnemonic == "WELL") {
                    las_data_.well_name = value;
                    well_name_edit_->setText(value);
                } else if (mnemonic == "FLD" || mnemonic == "FIELD") {
                    las_data_.field = value;
                    field_edit_->setText(value);
                } else if (mnemonic == "UWI" || mnemonic == "UWID") {
                    las_data_.uwi = value;
                    uwi_edit_->setText(value);
                } else if (mnemonic == "NULL") {
                    bool ok;
                    double null_val = value.toDouble(&ok);
                    if (ok) {
                        las_data_.null_value = null_val;
                    }
                }
            }
        } else if (current_section == "C") {
            // Curve Information Section
            QRegularExpression re(R"(^(\w+)\s*\.)");
            auto match = re.match(line);
            if (match.hasMatch()) {
                QString mnemonic = match.captured(1).toUpper();
                las_data_.curve_names.append(mnemonic);
                ++curve_count;
            }
        } else if (in_data_section) {
            // ASCII Data Section
            QStringList values = line.split(QRegularExpression("\\s+"), Qt::SkipEmptyParts);

            // Инициализация массивов данных при первой строке
            if (las_data_.curve_data.isEmpty()) {
                las_data_.curve_data.resize(curve_count);
            }

            // Чтение значений
            for (int i = 0; i < values.size() && i < curve_count; ++i) {
                bool ok;
                double val = values[i].toDouble(&ok);
                if (ok) {
                    las_data_.curve_data[i].append(val);
                } else {
                    las_data_.curve_data[i].append(las_data_.null_value);
                }
            }
        }
    }

    file.close();

    log_text_->append(tr("Загружено кривых: %1").arg(curve_count));
    if (!las_data_.curve_data.isEmpty()) {
        log_text_->append(tr("Точек данных: %1").arg(las_data_.curve_data[0].size()));
    }
}

void ImportLasDialog::populateCurveComboBoxes() {
    depth_curve_combo_->clear();
    angle_curve_combo_->clear();
    azimuth_curve_combo_->clear();

    // Добавляем пустой элемент для необязательного азимута
    azimuth_curve_combo_->addItem(tr("(не выбрано)"), -1);

    int depth_idx = -1;
    int angle_idx = -1;
    int azimuth_idx = -1;

    for (int i = 0; i < las_data_.curve_names.size(); ++i) {
        QString name = las_data_.curve_names[i];
        depth_curve_combo_->addItem(name, i);
        angle_curve_combo_->addItem(name, i);
        azimuth_curve_combo_->addItem(name, i);

        // Автоопределение кривых по названию
        QString upper = name.toUpper();
        if (depth_idx < 0 && (upper == "DEPT" || upper == "DEPTH" || upper == "MD" ||
                              upper == "MDEP" || upper == "MEASURED_DEPTH")) {
            depth_idx = i;
        }
        if (angle_idx < 0 && (upper == "INCL" || upper == "INC" || upper == "ANGLE" ||
                              upper == "DEVI" || upper == "DEVIATION" || upper == "ZEN")) {
            angle_idx = i;
        }
        if (azimuth_idx < 0 && (upper == "AZIM" || upper == "AZ" || upper == "AZIMUTH" ||
                                upper == "HAZI" || upper == "MTF" || upper == "MAGAZ")) {
            azimuth_idx = i;
        }
    }

    // Установка автоопределённых кривых
    if (depth_idx >= 0) {
        depth_curve_combo_->setCurrentIndex(depth_idx);
    }
    if (angle_idx >= 0) {
        angle_curve_combo_->setCurrentIndex(angle_idx);
    }
    if (azimuth_idx >= 0) {
        azimuth_curve_combo_->setCurrentIndex(azimuth_idx + 1);  // +1 из-за "(не выбрано)"
    }
}

void ImportLasDialog::onCurveSelectionChanged() {
    updatePreview();
}

void ImportLasDialog::onPreviewData() {
    updatePreview();
}

void ImportLasDialog::updatePreview() {
    preview_table_->setRowCount(0);

    if (las_data_.curve_data.isEmpty()) {
        points_count_label_->setText(tr("Точек: 0"));
        return;
    }

    int depth_idx = depth_curve_combo_->currentData().toInt();
    int angle_idx = angle_curve_combo_->currentData().toInt();
    int azimuth_idx = azimuth_curve_combo_->currentData().toInt();

    if (depth_idx < 0 || angle_idx < 0) {
        log_text_->append(tr("Предупреждение: не выбраны кривые глубины или угла"));
        return;
    }

    const auto& depth_data = las_data_.curve_data[depth_idx];
    const auto& angle_data = las_data_.curve_data[angle_idx];
    const QVector<double>* azimuth_data = (azimuth_idx >= 0) ? &las_data_.curve_data[azimuth_idx] : nullptr;

    bool angle_degmin = angle_degmin_check_->isChecked();
    bool azimuth_degmin = azimuth_degmin_check_->isChecked();

    int valid_count = 0;
    int max_preview = 100;  // Ограничение предпросмотра

    for (int i = 0; i < depth_data.size() && preview_table_->rowCount() < max_preview; ++i) {
        double depth = depth_data[i];
        double angle = angle_data[i];
        double azimuth = (azimuth_data && i < azimuth_data->size()) ? (*azimuth_data)[i] : las_data_.null_value;

        // Пропуск NULL-значений
        bool depth_valid = (std::abs(depth - las_data_.null_value) > 0.001);
        bool angle_valid = (std::abs(angle - las_data_.null_value) > 0.001);
        bool azimuth_valid = azimuth_data && (std::abs(azimuth - las_data_.null_value) > 0.001);

        if (!depth_valid || !angle_valid) {
            continue;
        }

        // Конвертация из градусы.минуты если нужно
        if (angle_degmin) {
            angle = utils::deg_from_degmin(angle);
        }
        if (azimuth_degmin && azimuth_valid) {
            azimuth = utils::deg_from_degmin(azimuth);
        }

        int row = preview_table_->rowCount();
        preview_table_->insertRow(row);

        preview_table_->setItem(row, 0, new QTableWidgetItem(QString::number(depth, 'f', 2)));
        preview_table_->setItem(row, 1, new QTableWidgetItem(QString::number(angle, 'f', 2)));

        if (azimuth_valid) {
            preview_table_->setItem(row, 2, new QTableWidgetItem(QString::number(azimuth, 'f', 2)));
        } else {
            preview_table_->setItem(row, 2, new QTableWidgetItem("-"));
        }

        QString status;
        if (angle < 0 || angle > 120) {
            status = tr("Ошибка: угол вне диапазона");
        } else if (azimuth_valid && (azimuth < 0 || azimuth > 360)) {
            status = tr("Предупреждение: азимут вне диапазона");
        } else {
            status = tr("OK");
        }
        preview_table_->setItem(row, 3, new QTableWidgetItem(status));

        ++valid_count;
    }

    points_count_label_->setText(tr("Точек: %1 (показано первых %2)").arg(valid_count).arg(preview_table_->rowCount()));
}

void ImportLasDialog::onImport() {
    if (las_data_.curve_data.isEmpty()) {
        QMessageBox::warning(this, tr("Импорт"),
                             tr("Нет данных для импорта. Загрузите LAS-файл."));
        return;
    }

    int depth_idx = depth_curve_combo_->currentData().toInt();
    int angle_idx = angle_curve_combo_->currentData().toInt();
    int azimuth_idx = azimuth_curve_combo_->currentData().toInt();

    if (depth_idx < 0 || angle_idx < 0) {
        QMessageBox::warning(this, tr("Импорт"),
                             tr("Необходимо выбрать кривые глубины и угла."));
        return;
    }

    // Заполнение метаданных
    well_->metadata.well_name = well_name_edit_->text().toStdString();
    well_->metadata.field_name = field_edit_->text().toStdString();
    well_->metadata.uwi = uwi_edit_->text().toStdString();
    well_->metadata.file_name = file_path_.toStdString();
    well_->source_file_path = file_path_.toStdString();
    well_->source_format = "las";

    // Очистка предыдущих измерений
    well_->measurements.clear();

    const auto& depth_data = las_data_.curve_data[depth_idx];
    const auto& angle_data = las_data_.curve_data[angle_idx];
    const QVector<double>* azimuth_data = (azimuth_idx >= 0) ? &las_data_.curve_data[azimuth_idx] : nullptr;

    bool angle_degmin = angle_degmin_check_->isChecked();
    bool azimuth_degmin = azimuth_degmin_check_->isChecked();

    int imported = 0;
    int skipped = 0;

    for (int i = 0; i < depth_data.size(); ++i) {
        double depth = depth_data[i];
        double angle = angle_data[i];
        double azimuth = (azimuth_data && i < azimuth_data->size()) ? (*azimuth_data)[i] : las_data_.null_value;

        // Пропуск NULL-значений
        bool depth_valid = (std::abs(depth - las_data_.null_value) > 0.001);
        bool angle_valid = (std::abs(angle - las_data_.null_value) > 0.001);
        bool azimuth_valid = azimuth_data && (std::abs(azimuth - las_data_.null_value) > 0.001);

        if (!depth_valid || !angle_valid) {
            ++skipped;
            continue;
        }

        // Конвертация из градусы.минуты если нужно
        if (angle_degmin) {
            angle = utils::deg_from_degmin(angle);
        }
        if (azimuth_degmin && azimuth_valid) {
            azimuth = utils::deg_from_degmin(azimuth);
        }

        models::MeasuredPoint point;
        point.measured_depth_m = depth;
        point.inclination_deg = angle;
        if (azimuth_valid) {
            point.azimuth_deg = azimuth;
            point.azimuth_type = models::AzimuthType::kMagnetic;
        }

        well_->measurements.push_back(point);
        ++imported;
    }

    if (imported == 0) {
        QMessageBox::warning(this, tr("Импорт"),
                             tr("Не удалось импортировать данные. Проверьте выбор кривых."));
        return;
    }

    import_successful_ = true;
    LOG_INFO(tr("Импортировано из LAS: %1 точек, пропущено: %2").arg(imported).arg(skipped));

    QMessageBox::information(this, tr("Импорт"),
                             tr("Импортировано точек: %1\nПропущено: %2").arg(imported).arg(skipped));

    accept();
}

std::shared_ptr<models::WellData> ImportLasDialog::wellData() const {
    return well_;
}

}  // namespace incline3d::ui

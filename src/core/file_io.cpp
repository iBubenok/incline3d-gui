#include "core/file_io.h"

#include <QFile>
#include <QFileInfo>
#include <QTextStream>
#include <QRegularExpression>
#include <QProcess>
#include <QTemporaryFile>

namespace incline3d::core {

FileFormat FileIO::detectFormat(const QString& path) {
    QString ext = QFileInfo(path).suffix().toLower();
    if (ext == "csv") return FileFormat::kCsv;
    if (ext == "las") return FileFormat::kLas;
    if (ext == "zak") return FileFormat::kZak;
    if (ext == "ws" || ext == "txt") return FileFormat::kWs;
    return FileFormat::kUnknown;
}

QString FileIO::formatToString(FileFormat format) {
    switch (format) {
        case FileFormat::kCsv: return "csv";
        case FileFormat::kLas: return "las";
        case FileFormat::kZak: return "zak";
        case FileFormat::kWs: return "ws";
        default: return "unknown";
    }
}

FileFormat FileIO::stringToFormat(const QString& str) {
    QString lower = str.toLower();
    if (lower == "csv") return FileFormat::kCsv;
    if (lower == "las") return FileFormat::kLas;
    if (lower == "zak") return FileFormat::kZak;
    if (lower == "ws") return FileFormat::kWs;
    return FileFormat::kUnknown;
}

QString FileIO::getOpenFileFilter() {
    return QObject::tr(
        "Все поддерживаемые (*.csv *.las *.zak *.ws *.txt);;"
        "CSV файлы (*.csv);;"
        "LAS файлы (*.las);;"
        "ZAK файлы (*.zak);;"
        "WS файлы (*.ws *.txt);;"
        "Все файлы (*)");
}

QString FileIO::getSaveFileFilter() {
    return QObject::tr(
        "CSV файлы (*.csv);;"
        "LAS файлы (*.las);;"
        "ZAK файлы (*.zak);;"
        "WS файлы (*.ws);;"
        "Все файлы (*)");
}

void FileIO::setInclprocPath(const QString& path) {
    inclproc_path_ = path;
}

WellLoadResult FileIO::loadWell(const QString& path, FileFormat format) {
    WellLoadResult result;

    if (format == FileFormat::kUnknown) {
        format = detectFormat(path);
    }

    if (format == FileFormat::kUnknown) {
        result.error_message = QObject::tr("Неизвестный формат файла: %1").arg(path);
        return result;
    }

    // Для WS-файлов парсим напрямую
    if (format == FileFormat::kWs) {
        return parseWsFile(path);
    }

    // Для CSV пробуем простой парсинг
    if (format == FileFormat::kCsv) {
        return parseCsvMeasurements(path);
    }

    // Для остальных форматов нужен inclproc для конвертации
    result.error_message = QObject::tr(
        "Для формата %1 требуется конвертация через inclproc").arg(formatToString(format));
    return result;
}

LoadResult FileIO::saveWell(const QString& path, const models::WellData& well, FileFormat format) {
    LoadResult result;

    if (format == FileFormat::kUnknown) {
        format = detectFormat(path);
    }

    if (format == FileFormat::kWs) {
        if (writeWsFile(path, well)) {
            result.success = true;
        } else {
            result.error_message = QObject::tr("Не удалось записать файл: %1").arg(path);
        }
        return result;
    }

    result.error_message = QObject::tr(
        "Сохранение в формат %1 пока не реализовано").arg(formatToString(format));
    return result;
}

WellLoadResult FileIO::parseWsFile(const QString& path) {
    WellLoadResult result;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        result.error_message = QObject::tr("Не удалось открыть файл: %1").arg(path);
        return result;
    }

    result.well = std::make_shared<models::WellData>();
    result.well->source_file_path = path.toStdString();
    result.well->source_format = "ws";

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);

    QString current_section;
    QStringList headers;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        // Пропуск пустых строк и комментариев
        if (line.isEmpty() || line.startsWith('#') || line.startsWith(';')) {
            continue;
        }

        // Определение секции
        if (line.startsWith('[') && line.endsWith(']')) {
            current_section = line.mid(1, line.length() - 2).toLower();
            headers.clear();
            continue;
        }

        // Парсинг заголовков (первая строка после секции)
        if (headers.isEmpty() && !current_section.isEmpty()) {
            headers = line.split('\t', Qt::KeepEmptyParts);
            continue;
        }

        QStringList values = line.split('\t', Qt::KeepEmptyParts);

        // Секция intervals (исходные замеры)
        if (current_section == "intervals") {
            if (values.size() >= 3) {
                models::MeasuredPoint point;
                bool ok1, ok2;
                point.measured_depth_m = values[0].toDouble(&ok1);
                point.inclination_deg = values[1].toDouble(&ok2);

                if (ok1 && ok2) {
                    if (values.size() >= 4 && !values[2].isEmpty()) {
                        bool ok3;
                        double azim = values[2].toDouble(&ok3);
                        if (ok3) {
                            point.azimuth_deg = azim;
                        }
                    }
                    result.well->measurements.push_back(point);
                }
            }
        }
        // Секция results (результаты расчёта)
        else if (current_section == "results") {
            if (values.size() >= 7) {
                models::ProcessedPoint point;
                point.measured_depth_m = values[0].toDouble();
                point.inclination_deg = values[1].toDouble();
                if (!values[2].isEmpty()) {
                    point.azimuth_deg = values[2].toDouble();
                }
                point.applied_azimuth_deg = values[3].toDouble();
                point.north_m = values[4].toDouble();
                point.east_m = values[5].toDouble();
                point.tvd_m = values[6].toDouble();

                if (values.size() >= 11) {
                    point.dogleg_angle_deg = values[7].toDouble();
                    point.intensity_10m = values[8].toDouble();
                    point.intensity_L = values[9].toDouble();
                }

                if (values.size() >= 15) {
                    point.mistake_x = values[10].toDouble();
                    point.mistake_y = values[11].toDouble();
                    point.mistake_z = values[12].toDouble();
                    point.mistake_absg = values[13].toDouble();
                }

                result.well->results.push_back(point);
            }
        }
        // Секция metadata
        else if (current_section == "metadata" || current_section == "well") {
            if (values.size() >= 2) {
                QString key = values[0].toLower();
                QString value = values[1];
                if (key == "well_name" || key == "name") {
                    result.well->metadata.well_name = value.toStdString();
                } else if (key == "field" || key == "field_name") {
                    result.well->metadata.field_name = value.toStdString();
                } else if (key == "cluster" || key == "well_pad") {
                    result.well->metadata.well_pad = value.toStdString();
                } else if (key == "uwi") {
                    result.well->metadata.uwi = value.toStdString();
                }
            }
        }
    }

    // Вычисление сводных данных
    if (!result.well->results.empty()) {
        double max_incl = 0.0;
        double max_int = 0.0;
        double max_int_depth = 0.0;

        for (const auto& pt : result.well->results) {
            if (pt.inclination_deg > max_incl) {
                max_incl = pt.inclination_deg;
            }
            if (pt.intensity_10m > max_int) {
                max_int = pt.intensity_10m;
                max_int_depth = pt.measured_depth_m;
            }
        }

        result.well->max_inclination_deg = max_incl;
        result.well->max_intensity_10m = max_int;
        result.well->max_intensity_10m_depth = max_int_depth;

        const auto& last = result.well->results.back();
        result.well->total_depth = last.measured_depth_m;
        result.well->horizontal_displacement = std::sqrt(
            last.north_m * last.north_m + last.east_m * last.east_m);
    } else if (!result.well->measurements.empty()) {
        result.well->total_depth = result.well->measurements.back().measured_depth_m;
    }

    // Имя скважины из файла если не задано
    if (result.well->metadata.well_name.empty()) {
        result.well->metadata.well_name = QFileInfo(path).baseName().toStdString();
    }

    result.success = true;
    return result;
}

WellLoadResult FileIO::parseCsvMeasurements(const QString& path) {
    WellLoadResult result;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        result.error_message = QObject::tr("Не удалось открыть файл: %1").arg(path);
        return result;
    }

    result.well = std::make_shared<models::WellData>();
    result.well->source_file_path = path.toStdString();
    result.well->source_format = "csv";
    result.well->metadata.well_name = QFileInfo(path).baseName().toStdString();

    QTextStream in(&file);
    in.setEncoding(QStringConverter::Utf8);

    bool first_line = true;
    int depth_col = -1, incl_col = -1, azim_col = -1;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();

        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }

        // Разделитель: точка с запятой или табуляция
        QStringList values;
        if (line.contains(';')) {
            values = line.split(';', Qt::KeepEmptyParts);
        } else if (line.contains('\t')) {
            values = line.split('\t', Qt::KeepEmptyParts);
        } else {
            values = line.split(',', Qt::KeepEmptyParts);
        }

        // Определение колонок из заголовка
        if (first_line) {
            first_line = false;
            for (int i = 0; i < values.size(); ++i) {
                QString h = values[i].toLower().trimmed();
                if (h.contains("глубина") || h.contains("depth") || h == "md") {
                    depth_col = i;
                } else if (h.contains("угол") || h.contains("incl") || h.contains("angle")) {
                    incl_col = i;
                } else if (h.contains("азимут") || h.contains("azim")) {
                    azim_col = i;
                }
            }

            // Если не нашли заголовки, предполагаем стандартный порядок
            if (depth_col < 0 || incl_col < 0) {
                // Пробуем парсить как данные
                bool ok1, ok2;
                if (values.size() >= 2) {
                    values[0].toDouble(&ok1);
                    values[1].toDouble(&ok2);
                    if (ok1 && ok2) {
                        // Это данные, не заголовок
                        depth_col = 0;
                        incl_col = 1;
                        azim_col = values.size() >= 3 ? 2 : -1;
                        // Не пропускаем эту строку - она содержит данные
                    } else {
                        depth_col = 0;
                        incl_col = 1;
                        azim_col = values.size() >= 3 ? 2 : -1;
                        continue;
                    }
                }
            } else {
                continue;  // Пропускаем строку заголовков
            }
        }

        if (depth_col < 0 || incl_col < 0 ||
            depth_col >= values.size() || incl_col >= values.size()) {
            continue;
        }

        models::MeasuredPoint point;
        bool ok1, ok2;
        point.measured_depth_m = values[depth_col].trimmed().toDouble(&ok1);
        point.inclination_deg = values[incl_col].trimmed().toDouble(&ok2);

        if (!ok1 || !ok2) {
            continue;
        }

        if (azim_col >= 0 && azim_col < values.size() && !values[azim_col].trimmed().isEmpty()) {
            bool ok3;
            double azim = values[azim_col].trimmed().toDouble(&ok3);
            if (ok3) {
                point.azimuth_deg = azim;
            }
        }

        result.well->measurements.push_back(point);
    }

    if (result.well->measurements.empty()) {
        result.error_message = QObject::tr("Не удалось прочитать данные из файла");
        return result;
    }

    result.well->total_depth = result.well->measurements.back().measured_depth_m;
    result.success = true;
    return result;
}

bool FileIO::writeWsFile(const QString& path, const models::WellData& well) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    out.setEncoding(QStringConverter::Utf8);

    // Секция метаданных
    out << "[metadata]\n";
    out << "well_name\t" << QString::fromStdString(well.metadata.well_name) << "\n";
    if (!well.metadata.field_name.empty()) {
        out << "field_name\t" << QString::fromStdString(well.metadata.field_name) << "\n";
    }
    if (!well.metadata.well_pad.empty()) {
        out << "well_pad\t" << QString::fromStdString(well.metadata.well_pad) << "\n";
    }
    if (!well.metadata.uwi.empty()) {
        out << "uwi\t" << QString::fromStdString(well.metadata.uwi) << "\n";
    }
    out << "\n";

    // Секция исходных замеров
    if (!well.measurements.empty()) {
        out << "[intervals]\n";
        out << "Глубина_м\tУгол_град\tАзимут_град\n";
        for (const auto& pt : well.measurements) {
            out << QString::number(pt.measured_depth_m, 'f', 2) << "\t";
            out << QString::number(pt.inclination_deg, 'f', 2) << "\t";
            if (pt.azimuth_deg.has_value()) {
                out << QString::number(pt.azimuth_deg.value(), 'f', 2);
            }
            out << "\n";
        }
        out << "\n";
    }

    // Секция результатов
    if (!well.results.empty()) {
        out << "[results]\n";
        out << "Глубина_м\tУгол_град\tАзимут_град\tПрив_азимут\tСевер_м\tВосток_м\tTVD_м\t";
        out << "Доглег_град\tИнт10_град\tИнтL_град\tОшX_м\tОшY_м\tОшZ_м\tОшR_м\n";

        for (const auto& pt : well.results) {
            out << QString::number(pt.measured_depth_m, 'f', 2) << "\t";
            out << QString::number(pt.inclination_deg, 'f', 2) << "\t";
            if (pt.azimuth_deg.has_value()) {
                out << QString::number(pt.azimuth_deg.value(), 'f', 2);
            }
            out << "\t";
            out << QString::number(pt.applied_azimuth_deg, 'f', 2) << "\t";
            out << QString::number(pt.north_m, 'f', 2) << "\t";
            out << QString::number(pt.east_m, 'f', 2) << "\t";
            out << QString::number(pt.tvd_m, 'f', 2) << "\t";
            out << QString::number(pt.dogleg_angle_deg, 'f', 3) << "\t";
            out << QString::number(pt.intensity_10m, 'f', 2) << "\t";
            out << QString::number(pt.intensity_L, 'f', 2) << "\t";
            out << QString::number(pt.mistake_x, 'f', 3) << "\t";
            out << QString::number(pt.mistake_y, 'f', 3) << "\t";
            out << QString::number(pt.mistake_z, 'f', 3) << "\t";
            out << QString::number(pt.mistake_absg, 'f', 3) << "\n";
        }
    }

    return true;
}

std::vector<models::ProjectPoint> FileIO::loadProjectPoints(const QString& path) {
    std::vector<models::ProjectPoint> points;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return points;
    }

    QTextStream in(&file);
    bool first_line = true;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }

        QStringList values = line.split('\t', Qt::KeepEmptyParts);
        if (values.isEmpty()) {
            values = line.split(';', Qt::KeepEmptyParts);
        }

        if (first_line) {
            first_line = false;
            // Пропускаем заголовок если он есть
            bool is_header = false;
            for (const auto& v : values) {
                if (v.toLower().contains("пласт") || v.toLower().contains("name")) {
                    is_header = true;
                    break;
                }
            }
            if (is_header) continue;
        }

        if (values.size() >= 5) {
            models::ProjectPoint pt;
            pt.name = values[0].toStdString();
            pt.azimuth_geogr_deg = values[1].toDouble();
            pt.shift_m = values[2].toDouble();
            pt.depth_m = values[3].toDouble();
            pt.radius_m = values[4].toDouble();
            if (values.size() >= 6) {
                pt.abs_depth_m = values[5].toDouble();
            }
            points.push_back(pt);
        }
    }

    return points;
}

bool FileIO::saveProjectPoints(const QString& path, const std::vector<models::ProjectPoint>& points) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    out << "Пласт\tАзимут_град\tСмещение_м\tГлубина_м\tРадиус_м\tАбс_глубина_м\n";

    for (const auto& pt : points) {
        out << QString::fromStdString(pt.name) << "\t";
        out << QString::number(pt.azimuth_geogr_deg, 'f', 2) << "\t";
        out << QString::number(pt.shift_m, 'f', 1) << "\t";
        out << QString::number(pt.depth_m, 'f', 1) << "\t";
        out << QString::number(pt.radius_m, 'f', 1) << "\t";
        out << QString::number(pt.abs_depth_m, 'f', 1) << "\n";
    }

    return true;
}

std::vector<models::ShotPoint> FileIO::loadShotPoints(const QString& path) {
    std::vector<models::ShotPoint> points;

    QFile file(path);
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        return points;
    }

    QTextStream in(&file);
    bool first_line = true;

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith('#')) {
            continue;
        }

        QStringList values = line.split('\t', Qt::KeepEmptyParts);
        if (values.isEmpty()) {
            values = line.split(';', Qt::KeepEmptyParts);
        }

        if (first_line) {
            first_line = false;
            bool is_header = false;
            for (const auto& v : values) {
                QString lower = v.toLower();
                if (lower.contains("name") || lower.contains("название") ||
                    lower == "x" || lower == "y") {
                    is_header = true;
                    break;
                }
            }
            if (is_header) continue;
        }

        if (values.size() >= 4) {
            models::ShotPoint pt;
            pt.name = values[0].toStdString();
            pt.x_m = values[1].toDouble();
            pt.y_m = values[2].toDouble();
            pt.z_m = values[3].toDouble();
            points.push_back(pt);
        }
    }

    return points;
}

bool FileIO::saveShotPoints(const QString& path, const std::vector<models::ShotPoint>& points) {
    QFile file(path);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        return false;
    }

    QTextStream out(&file);
    out << "Название\tX_м\tY_м\tZ_м\n";

    for (const auto& pt : points) {
        out << QString::fromStdString(pt.name) << "\t";
        out << QString::number(pt.x_m, 'f', 1) << "\t";
        out << QString::number(pt.y_m, 'f', 1) << "\t";
        out << QString::number(pt.z_m, 'f', 1) << "\n";
    }

    return true;
}

}  // namespace incline3d::core

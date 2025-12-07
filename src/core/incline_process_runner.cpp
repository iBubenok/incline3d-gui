#include "core/incline_process_runner.h"

#include <QCoreApplication>
#include <QDir>
#include <QFileInfo>
#include <QRegularExpression>

namespace incline3d::core {

InclineProcessRunner::InclineProcessRunner(QObject* parent)
    : QObject(parent) {
    // Путь по умолчанию
#ifdef INCLPROC_DEFAULT_PATH
    inclproc_path_ = QString::fromUtf8(INCLPROC_DEFAULT_PATH);
#else
    // Пытаемся найти inclproc рядом с приложением
    QString app_dir = QCoreApplication::applicationDirPath();
    QStringList candidates = {
        app_dir + "/inclproc",
        app_dir + "/../incline3d-cpp20/build/inclproc",
        app_dir + "/../../incline3d-cpp20/build/inclproc",
        "/usr/local/bin/inclproc",
        "/usr/bin/inclproc"
    };

    for (const auto& path : candidates) {
        if (QFileInfo::exists(path)) {
            inclproc_path_ = path;
            break;
        }
    }
#endif
}

InclineProcessRunner::~InclineProcessRunner() {
    cancel();
}

void InclineProcessRunner::setInclprocPath(const QString& path) {
    inclproc_path_ = path;
}

QString InclineProcessRunner::inclprocPath() const {
    return inclproc_path_;
}

bool InclineProcessRunner::isInclprocAvailable() const {
    return QFileInfo::exists(inclproc_path_) && QFileInfo(inclproc_path_).isExecutable();
}

QStringList InclineProcessRunner::buildProcessArgs(
    const QString& input_file, const QString& input_format,
    const QString& output_file, const QString& output_format,
    const models::CalculationParams& params) const {

    QStringList args;
    args << "process";
    args << "--input" << input_file;
    args << "--input-format" << input_format;
    args << "--output" << output_file;
    args << "--output-format" << output_format;

    // Метод расчёта
    args << "--method" << QString::fromStdString(models::method_to_string(params.method));

    // Поправки
    if (params.magnetic_declination_deg != 0.0) {
        args << "--declination" << QString::number(params.magnetic_declination_deg);
    }
    if (params.meridian_convergence_deg != 0.0) {
        args << "--meridian" << QString::number(params.meridian_convergence_deg);
    }

    // Интервал интенсивности
    if (params.intensity_interval_m != 30.0) {
        args << "--intensity-step" << QString::number(params.intensity_interval_m);
    }

    // Погрешности замеров
    if (params.error_depth_m != 0.1) {
        args << "--err-depth" << QString::number(params.error_depth_m);
    }
    if (params.error_inclination_deg != 0.1) {
        args << "--err-angle" << QString::number(params.error_inclination_deg);
    }
    if (params.error_azimuth_deg != 0.1) {
        args << "--err-azim" << QString::number(params.error_azimuth_deg);
    }

    // Пороги предупреждений
    if (params.intensity_threshold_deg > 0) {
        args << "--intensity-threshold" << QString::number(params.intensity_threshold_deg);
    }
    if (params.delta_depth_warning_m > 0) {
        args << "--delta-depth-warning" << QString::number(params.delta_depth_warning_m);
    }

    // Интерполяция
    if (params.interpolation_step_m > 0) {
        args << "--interp-step" << QString::number(params.interpolation_step_m);
    }

    // Тип азимута
    QString azimuth_mode;
    switch (params.azimuth_type) {
        case models::AzimuthType::kMagnetic:
            azimuth_mode = "magnetic";
            break;
        case models::AzimuthType::kTrue:
            azimuth_mode = "true";
            break;
        case models::AzimuthType::kGrid:
            azimuth_mode = "grid";
            break;
    }
    args << "--azimuth" << azimuth_mode;

    // Опции обработки азимутов
    if (!params.use_last_azimuth) {
        args << "--no-use-last-azimuth";
    }
    if (!params.interpolate_missing_azimuths) {
        args << "--no-interp-azimuths";
    }
    if (!params.unwrap_azimuths) {
        args << "--no-unwrap-azimuths";
    }

    // Сглаживание интенсивности
    if (params.smooth_intensity) {
        args << "--smooth-intensity";
    }

    // SNGF-режим
    if (params.sngf_mode) {
        args << "--sngf-mode";
        if (params.sngf_min_angle_deg != 5.0) {
            args << "--sngf-min-angle" << QString::number(params.sngf_min_angle_deg);
        }
    }

    // Высотные отметки
    if (params.kelly_bushing_elevation_m != 0.0) {
        args << "--kelly-bushing" << QString::number(params.kelly_bushing_elevation_m);
    }
    if (params.ground_elevation_m != 0.0) {
        args << "--ground-elevation" << QString::number(params.ground_elevation_m);
    }
    if (params.water_depth_m != 0.0) {
        args << "--water-depth" << QString::number(params.water_depth_m);
    }

    // Контроль качества
    if (params.quality_check) {
        args << "--quality-check";
        if (params.max_angle_deviation_deg != 5.0) {
            args << "--max-angle-deviation" << QString::number(params.max_angle_deviation_deg);
        }
        if (params.max_azimuth_deviation_deg != 10.0) {
            args << "--max-azimuth-deviation" << QString::number(params.max_azimuth_deviation_deg);
        }
    }

    return args;
}

ProcessResult InclineProcessRunner::runProcess(ProcessCommand cmd, const QStringList& args) {
    ProcessResult result;

    if (!isInclprocAvailable()) {
        result.error_message = tr("Исполняемый файл inclproc не найден: %1").arg(inclproc_path_);
        return result;
    }

    QProcess process;
    process.setProgram(inclproc_path_);
    process.setArguments(args);

    process.start();

    if (!process.waitForStarted(5000)) {
        result.error_message = tr("Не удалось запустить inclproc: %1").arg(process.errorString());
        return result;
    }

    // Ожидание завершения (таймаут 5 минут для больших файлов)
    if (!process.waitForFinished(300000)) {
        process.kill();
        result.error_message = tr("Превышено время ожидания выполнения inclproc");
        return result;
    }

    result.exit_code = process.exitCode();
    result.stdout_output = QString::fromUtf8(process.readAllStandardOutput());
    result.stderr_output = QString::fromUtf8(process.readAllStandardError());

    // Интерпретация кодов возврата
    switch (result.exit_code) {
        case 0:
            result.success = true;
            break;
        case 1:
            result.error_message = tr("Ошибка входных данных или аргументов");
            break;
        case 2:
            result.error_message = tr("Ошибка расчёта");
            break;
        case 3:
            result.error_message = tr("Ошибка ввода/вывода");
            break;
        case 4:
            result.error_message = tr("Выход за допуск сближения");
            result.success = true;  // Расчёт выполнен, но с предупреждением
            break;
        default:
            result.error_message = tr("Неизвестная ошибка (код %1)").arg(result.exit_code);
            break;
    }

    // Добавляем stderr к сообщению об ошибке
    if (!result.success && !result.stderr_output.isEmpty()) {
        result.error_message += "\n" + result.stderr_output;
    }

    return result;
}

ProcessResult InclineProcessRunner::process(
    const QString& input_file, const QString& input_format,
    const QString& output_file, const QString& output_format,
    const models::CalculationParams& params) {

    QStringList args = buildProcessArgs(input_file, input_format,
                                        output_file, output_format, params);
    return runProcess(ProcessCommand::kProcess, args);
}

ProcessResult InclineProcessRunner::convert(
    const QString& input_file, const QString& input_format,
    const QString& output_file, const QString& output_format) {

    QStringList args;
    args << "convert";
    args << "--input" << input_file;
    args << "--input-format" << input_format;
    args << "--output" << output_file;
    args << "--output-format" << output_format;

    return runProcess(ProcessCommand::kConvert, args);
}

ProcessResult InclineProcessRunner::report(
    const QString& input_file, const QString& input_format,
    const QString& output_file) {

    QStringList args;
    args << "report";
    args << "--input" << input_file;
    args << "--input-format" << input_format;
    args << "--output" << output_file;

    return runProcess(ProcessCommand::kReport, args);
}

ProcessResult InclineProcessRunner::proximity(
    const QString& file_a, const QString& format_a,
    const QString& file_b, const QString& format_b,
    double tolerance) {

    QStringList args;
    args << "proximity";
    args << "--input" << file_a;
    args << "--input-format" << format_a;
    args << "--input-b" << file_b;
    args << "--input-format-b" << format_b;

    if (tolerance > 0) {
        args << "--tolerance" << QString::number(tolerance);
    }

    ProcessResult result = runProcess(ProcessCommand::kProximity, args);
    if (result.success || result.exit_code == 4) {
        parseProximityOutput(result.stdout_output, result);
    }

    return result;
}

ProcessResult InclineProcessRunner::offset(
    const QString& file_a, const QString& format_a,
    const QString& file_b, const QString& format_b,
    double tvd) {

    QStringList args;
    args << "offset";
    args << "--input" << file_a;
    args << "--input-format" << format_a;
    args << "--input-b" << file_b;
    args << "--input-format-b" << format_b;
    args << "--tvd" << QString::number(tvd);

    ProcessResult result = runProcess(ProcessCommand::kOffset, args);
    if (result.success) {
        parseOffsetOutput(result.stdout_output, result);
    }

    return result;
}

void InclineProcessRunner::parseProximityOutput(const QString& output, ProcessResult& result) {
    // Ищем строку вида "Минимальная дистанция: X.XX м"
    QRegularExpression re(R"((?:Минимальная\s+дистанция|Min\s+distance)[:\s]+([0-9.]+))");
    QRegularExpressionMatch match = re.match(output);
    if (match.hasMatch()) {
        bool ok = false;
        double value = match.captured(1).toDouble(&ok);
        if (ok) {
            result.min_distance = value;
        }
    }
}

void InclineProcessRunner::parseOffsetOutput(const QString& output, ProcessResult& result) {
    // Ищем строку вида "Горизонтальный отход: X.XX м"
    QRegularExpression re(R"((?:Горизонтальный\s+отход|Horizontal\s+offset)[:\s]+([0-9.]+))");
    QRegularExpressionMatch match = re.match(output);
    if (match.hasMatch()) {
        bool ok = false;
        double value = match.captured(1).toDouble(&ok);
        if (ok) {
            result.horizontal_offset = value;
        }
    }
}

void InclineProcessRunner::processAsync(
    const QString& input_file, const QString& input_format,
    const QString& output_file, const QString& output_format,
    const models::CalculationParams& params) {

    if (isRunning()) {
        emit errorOccurred(tr("Процесс уже выполняется"));
        return;
    }

    if (!isInclprocAvailable()) {
        emit errorOccurred(tr("Исполняемый файл inclproc не найден: %1").arg(inclproc_path_));
        return;
    }

    QStringList args = buildProcessArgs(input_file, input_format,
                                        output_file, output_format, params);

    current_process_ = std::make_unique<QProcess>();
    current_process_->setProgram(inclproc_path_);
    current_process_->setArguments(args);

    connect(current_process_.get(),
            QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, [this](int exitCode, QProcess::ExitStatus) {
                ProcessResult result;
                result.exit_code = exitCode;
                result.stdout_output = QString::fromUtf8(current_process_->readAllStandardOutput());
                result.stderr_output = QString::fromUtf8(current_process_->readAllStandardError());

                switch (exitCode) {
                    case 0:
                        result.success = true;
                        break;
                    case 1:
                        result.error_message = tr("Ошибка входных данных или аргументов");
                        break;
                    case 2:
                        result.error_message = tr("Ошибка расчёта");
                        break;
                    case 3:
                        result.error_message = tr("Ошибка ввода/вывода");
                        break;
                    case 4:
                        result.error_message = tr("Выход за допуск сближения");
                        result.success = true;
                        break;
                    default:
                        result.error_message = tr("Неизвестная ошибка (код %1)").arg(exitCode);
                        break;
                }

                if (!result.success && !result.stderr_output.isEmpty()) {
                    result.error_message += "\n" + result.stderr_output;
                }

                emit processFinished(result);
                current_process_.reset();
            });

    connect(current_process_.get(), &QProcess::errorOccurred,
            this, [this](QProcess::ProcessError error) {
                QString errorMsg;
                switch (error) {
                    case QProcess::FailedToStart:
                        errorMsg = tr("Не удалось запустить процесс");
                        break;
                    case QProcess::Crashed:
                        errorMsg = tr("Процесс аварийно завершился");
                        break;
                    case QProcess::Timedout:
                        errorMsg = tr("Превышено время ожидания");
                        break;
                    default:
                        errorMsg = tr("Ошибка процесса");
                        break;
                }
                emit errorOccurred(errorMsg);
            });

    current_process_->start();
}

void InclineProcessRunner::cancel() {
    if (current_process_ && current_process_->state() != QProcess::NotRunning) {
        current_process_->kill();
        current_process_->waitForFinished(1000);
        current_process_.reset();
    }
}

bool InclineProcessRunner::isRunning() const {
    return current_process_ && current_process_->state() == QProcess::Running;
}

}  // namespace incline3d::core

#pragma once

#include <QObject>
#include <QProcess>
#include <QString>

#include <functional>
#include <memory>
#include <optional>

#include "models/well_data.h"

namespace incline3d::core {

/// Результат выполнения команды inclproc
struct ProcessResult {
    bool success{false};
    int exit_code{0};
    QString stdout_output;
    QString stderr_output;
    QString error_message;

    // Специфичные для proximity/offset
    std::optional<double> min_distance;
    std::optional<double> horizontal_offset;
};

/// Тип команды inclproc
enum class ProcessCommand {
    kProcess,       ///< Расчёт траектории
    kConvert,       ///< Конвертация форматов
    kReport,        ///< Генерация отчёта
    kProximity,     ///< Анализ сближения
    kOffset         ///< Расчёт отхода
};

/// Класс для запуска и управления процессом inclproc
class InclineProcessRunner : public QObject {
    Q_OBJECT

public:
    explicit InclineProcessRunner(QObject* parent = nullptr);
    ~InclineProcessRunner() override;

    /// Установить путь к исполняемому файлу inclproc
    void setInclprocPath(const QString& path);
    QString inclprocPath() const;

    /// Проверить доступность inclproc
    bool isInclprocAvailable() const;

    /// Запустить расчёт траектории
    /// @param input_file путь к входному файлу
    /// @param input_format формат входного файла (csv, las, zak, ws)
    /// @param output_file путь к выходному файлу
    /// @param output_format формат выходного файла
    /// @param params параметры расчёта
    /// @return результат выполнения
    ProcessResult process(const QString& input_file, const QString& input_format,
                          const QString& output_file, const QString& output_format,
                          const models::CalculationParams& params);

    /// Конвертировать файл
    ProcessResult convert(const QString& input_file, const QString& input_format,
                          const QString& output_file, const QString& output_format);

    /// Получить отчёт
    ProcessResult report(const QString& input_file, const QString& input_format,
                         const QString& output_file);

    /// Анализ сближения двух траекторий
    ProcessResult proximity(const QString& file_a, const QString& format_a,
                            const QString& file_b, const QString& format_b,
                            double tolerance = 0.0);

    /// Расчёт горизонтального отхода
    ProcessResult offset(const QString& file_a, const QString& format_a,
                         const QString& file_b, const QString& format_b,
                         double tvd);

    /// Асинхронный запуск расчёта
    void processAsync(const QString& input_file, const QString& input_format,
                      const QString& output_file, const QString& output_format,
                      const models::CalculationParams& params);

    /// Прервать выполнение текущего процесса
    void cancel();

    /// Проверить, выполняется ли процесс
    bool isRunning() const;

signals:
    /// Сигнал о завершении асинхронной операции
    void processFinished(const ProcessResult& result);

    /// Сигнал о прогрессе (если доступен)
    void progressUpdated(int percent, const QString& message);

    /// Сигнал об ошибке
    void errorOccurred(const QString& error);

private:
    QStringList buildProcessArgs(const QString& input_file, const QString& input_format,
                                 const QString& output_file, const QString& output_format,
                                 const models::CalculationParams& params) const;

    ProcessResult runProcess(ProcessCommand cmd, const QStringList& args);

    void parseProximityOutput(const QString& output, ProcessResult& result);
    void parseOffsetOutput(const QString& output, ProcessResult& result);

    QString inclproc_path_;
    std::unique_ptr<QProcess> current_process_;
};

}  // namespace incline3d::core

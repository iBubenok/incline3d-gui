#pragma once

#include <QString>
#include <memory>
#include <optional>
#include <vector>

#include "models/well_data.h"
#include "models/project_point.h"
#include "models/shot_point.h"

namespace incline3d::core {

/// Результат загрузки файла
struct LoadResult {
    bool success{false};
    QString error_message;
    std::vector<QString> warnings;
};

/// Результат загрузки данных скважины
struct WellLoadResult : LoadResult {
    std::shared_ptr<models::WellData> well;
};

/// Формат файла данных
enum class FileFormat {
    kUnknown,
    kCsv,
    kLas,
    kZak,
    kWs
};

/// Класс для работы с файлами данных инклинометрии
class FileIO {
public:
    FileIO() = default;

    /// Определить формат файла по расширению
    static FileFormat detectFormat(const QString& path);

    /// Получить строковое представление формата
    static QString formatToString(FileFormat format);

    /// Получить формат из строки
    static FileFormat stringToFormat(const QString& str);

    /// Получить фильтр для диалога открытия файла
    static QString getOpenFileFilter();

    /// Получить фильтр для диалога сохранения файла
    static QString getSaveFileFilter();

    /// Загрузить данные скважины из файла
    /// @note Использует inclproc convert для конвертации в WS, затем парсит WS
    WellLoadResult loadWell(const QString& path, FileFormat format = FileFormat::kUnknown);

    /// Сохранить данные скважины в файл
    LoadResult saveWell(const QString& path, const models::WellData& well,
                        FileFormat format = FileFormat::kUnknown);

    /// Загрузить проектные точки из текстового файла
    std::vector<models::ProjectPoint> loadProjectPoints(const QString& path);

    /// Сохранить проектные точки в текстовый файл
    bool saveProjectPoints(const QString& path, const std::vector<models::ProjectPoint>& points);

    /// Загрузить пункты возбуждения из текстового файла
    std::vector<models::ShotPoint> loadShotPoints(const QString& path);

    /// Сохранить пункты возбуждения в текстовый файл
    bool saveShotPoints(const QString& path, const std::vector<models::ShotPoint>& points);

    /// Установить путь к inclproc (для конвертации)
    void setInclprocPath(const QString& path);

private:
    /// Парсинг текстового WS-файла
    WellLoadResult parseWsFile(const QString& path);

    /// Запись данных в WS-формат
    bool writeWsFile(const QString& path, const models::WellData& well);

    /// Простой парсер CSV (исходные замеры)
    WellLoadResult parseCsvMeasurements(const QString& path);

    QString inclproc_path_;
};

}  // namespace incline3d::core

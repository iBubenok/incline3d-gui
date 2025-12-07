# Архитектура Incline3D GUI

## Обзор

Incline3D GUI построен по архитектуре Model-View с чётким разделением слоёв:

```
┌─────────────────────────────────────────────────────────────┐
│                      UI Layer (ui/)                         │
│  MainWindow, Dialogs, Dock Widgets                          │
├─────────────────────────────────────────────────────────────┤
│                    Views Layer (views/)                     │
│  View3DWidget, PlanView, VerticalView                       │
├─────────────────────────────────────────────────────────────┤
│                  Qt Models Layer (models/)                  │
│  WellTableModel, ProjectPointsModel, ShotPointsModel, etc.  │
├─────────────────────────────────────────────────────────────┤
│                    Core Layer (core/)                       │
│  ProjectManager, FileIO, InclineProcessRunner, Settings     │
├─────────────────────────────────────────────────────────────┤
│                  Data Models (models/)                      │
│  WellData, ProjectPoint, ShotPoint                          │
├─────────────────────────────────────────────────────────────┤
│                   Utils Layer (utils/)                      │
│  Logger, AngleUtils                                         │
└─────────────────────────────────────────────────────────────┘
                              │
                              ▼
                  ┌───────────────────┐
                  │    inclproc CLI   │
                  │ (incline3d-cpp20) │
                  └───────────────────┘
```

## Слои приложения

### 1. Data Models (`src/models/`)

Чистые C++-структуры без зависимости от Qt (кроме QColor для визуализации).

#### WellData (`well_data.h`)

```cpp
struct MeasuredPoint {
    double md_m;            // Глубина по стволу
    double inclination_deg; // Угол наклона
    double azimuth_deg;     // Азимут
    AzimuthType azimuth_type;
    bool quality_ok;
};

struct ProcessedPoint {
    // Входные данные
    double md_m, inclination_deg, azimuth_deg;
    // Расчётные координаты
    double tvd_m, north_m, east_m;
    // Замыкание
    double closure_m, closure_azimuth_deg;
    // Интенсивности
    double dogleg_deg_30m, build_rate_deg_30m, turn_rate_deg_30m;
    // Погрешности
    double dls_error_m, north_error_m, east_error_m, tvd_error_m;
};

struct WellData {
    WellMetadata metadata;
    CalculationParams params;
    std::vector<MeasuredPoint> measurements;
    std::vector<ProcessedPoint> results;
    // Визуализация
    bool visible;
    QColor display_color;
    float line_width;
};
```

#### ProjectPoint (`project_point.h`)

Проектные точки пластов с плановыми и фактическими координатами.

#### ShotPoint (`shot_point.h`)

Пункты возбуждения для сейсмических работ.

### 2. Qt Models (`src/models/`)

Адаптеры `QAbstractTableModel` для отображения данных в таблицах.

| Модель | Данные | Назначение |
|--------|--------|------------|
| `WellTableModel` | `WellData[]` | Список скважин с агрегированными показателями |
| `MeasurementsModel` | `MeasuredPoint[]` | Редактируемая таблица исходных данных |
| `ResultsModel` | `ProcessedPoint[]` | Таблица результатов (только чтение) |
| `ProjectPointsModel` | `ProjectPoint[]` | Проектные точки |
| `ShotPointsModel` | `ShotPoint[]` | Пункты возбуждения |

### 3. Core Layer (`src/core/`)

#### InclineProcessRunner

Управляет запуском CLI `inclproc` через `QProcess`:

```cpp
class InclineProcessRunner : public QObject {
    // Синхронный запуск
    ProcessResult runProcess(const ProcessParams& params);
    ProcessResult runConvert(const ConvertParams& params);

    // Асинхронный запуск
    void runProcessAsync(const ProcessParams& params);

signals:
    void processFinished(const ProcessResult& result);
    void progressChanged(int percent, const QString& message);
};
```

Коды возврата `inclproc`:
- `0` — успех
- `1` — ошибка аргументов CLI
- `2` — ошибка чтения файла
- `3` — ошибка вычисления
- `4` — ошибка записи файла

#### FileIO

Чтение/запись файлов данных:

```cpp
class FileIO {
    LoadResult loadWell(const QString& path);
    SaveResult saveWell(const QString& path, const WellData& well);

    static QString getOpenFileFilter();  // "*.ws;*.csv;*.las;*.zak"
    static QString getSaveFileFilter();
};
```

#### ProjectManager

Управление проектом:

```cpp
class ProjectManager : public QObject {
    void newProject();
    bool loadProject(const QString& path);
    bool saveProject(const QString& path = QString());
    bool exportProject(const QString& dir);

    void addWell(std::shared_ptr<WellData> well);
    void removeWell(int index);

    ProjectData& projectData();
    QVector<std::shared_ptr<WellData>>& wells();
    bool isDirty() const;

signals:
    void projectCreated();
    void projectLoaded();
    void projectSaved();
    void dirtyChanged(bool dirty);
    void wellsChanged();
};
```

#### Settings

Синглтон для настроек приложения (QSettings):

```cpp
class Settings {
    static Settings& instance();

    // Пути
    QString inclprocPath() const;
    QString lastOpenDirectory() const;
    QStringList recentFiles() const;

    // Параметры по умолчанию
    CalculationParams defaultCalculationParams() const;
    QColor defaultWellColor() const;

    // Автосохранение
    bool autoSaveEnabled() const;
    int autoSaveIntervalMinutes() const;

    // Восстановление сессии
    bool crashRecoveryEnabled() const;
    QString recoveryProjectPath() const;
    void setRecoveryProjectPath(const QString& path);
    void clearRecoveryData();
};
```

### 4. Views Layer (`src/views/`)

#### View3DWidget

OpenGL-виджет для 3D-визуализации:

```cpp
class View3DWidget : public QOpenGLWidget, protected QOpenGLFunctions {
    void setWellModel(WellTableModel* model);
    void setProjectPointsModel(ProjectPointsModel* model);
    void setShotPointsModel(ShotPointsModel* model);

    ViewSettings& settings();
    void resetView();

protected:
    void paintGL() override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void wheelEvent(QWheelEvent* event) override;

private:
    void drawGrid();
    void drawAxes();
    void drawWells();
    void drawProjectPoints();
    void drawShotPoints();
};
```

Система координат:
- X — восток (east)
- Y — север (north)
- Z — глубина вниз (TVD отрицательное)

#### PlanView

2D-вид горизонтальной проекции (QGraphicsView):

```cpp
class PlanView : public QGraphicsView {
    void setShowGrid(bool show);
    void setGridStep(double step);
    void fitToContent();
    void refresh();

protected:
    void drawBackground(QPainter* painter, const QRectF& rect) override;
};
```

#### VerticalView

2D-вид вертикальной проекции на заданный профиль:

```cpp
class VerticalView : public QGraphicsView {
    void setProfileAzimuth(double azimuth_deg);
    void autoFitAzimuth();  // Автоподбор азимута

signals:
    void profileAzimuthChanged(double azimuth_deg);
};
```

### 5. UI Layer (`src/ui/`)

#### Диалоги

| Диалог | Назначение |
|--------|------------|
| `SettingsDialog` | Общие настройки приложения |
| `ManualInputDialog` | Ручной ввод данных скважины (2 вкладки: метаданные и массив замеров) |
| `ImportLasDialog` | Импорт данных из LAS-файла с выбором кривых |
| `ImportZakDialog` | Импорт данных из текстового файла ЗАК с настройками парсинга |
| `ProcessDialog` | Параметры обработки скважины (4 вкладки: метод, азимуты, высотные отметки, качество) |
| `ProximityDialog` | Анализ сближения траекторий |
| `OffsetDialog` | Анализ отхода от проектных точек |
| `ViewOptionsDialog` | Настройки отображения видов |
| `VerticalSettingsDialog` | Настройки вертикальной проекции (масштаб, азимут, шапка) |
| `ConclusionDialog` | Формирование заключения (4 вкладки: шапка, результаты, проектные точки, сводка) |
| `ExportImageDialog` | Экспорт изображений в файл/буфер обмена |
| `ReportHeaderDialog` | Редактирование шапки отчёта |
| `AboutDialog` | Информация о программе |

#### MainWindow

Главное окно с док-виджетами:

```
┌──────────────────────────────────────────────────────────┐
│ Файл  Редактирование  Исходные данные  Обработка  Вид  Отчёты  Справка │
├──────────────────────────────────────────────────────────┤
│ 🗋 🗁 💾 │ 📂 │ ▶️ ▶▶ │ 🌐 📋 📊                          │
├────────────┬─────────────────────────────┬───────────────┤
│            │                             │               │
│ Скважины   │                             │ Замеры        │
│ ──────────-│    [3D] [План] [Верт]       │ ──────────    │
│ □ Скв-1    │                             │ MD  Inc  Az   │
│ □ Скв-2    │                             │ ... ... ...   │
│            │       Область               │               │
│ Пр.точки   │       визуализации          │ Результаты    │
│ ──────────-│                             │ ──────────    │
│ ● Точка1   │                             │ TVD N   E     │
│            │                             │ ... ... ...   │
│ ПВ         │                             │               │
│ ──────────-│                             │               │
│ ▲ ПВ-1     │                             │               │
├────────────┴─────────────────────────────┴───────────────┤
│ Готов                                      [====      ]  │
└──────────────────────────────────────────────────────────┘
```

## Потоки данных

### Загрузка файла

```
FileDialog → FileIO.loadWell() → WellData
    ↓
ProjectManager.addWell()
    ↓
WellTableModel.addWell()
    ↓
emit dataChanged() → UI обновляется
```

### Обработка скважины

```
ProcessDialog → InclineProcessRunner
    ↓
Создать временный WS-файл → inclproc process → Прочитать результат
    ↓
WellData.results = результаты
    ↓
ResultsModel.refresh() + Views.update()
```

### Сохранение проекта

```
MainWindow.onSaveProject()
    ↓
ProjectManager.saveProject()
    ↓
Сериализация ProjectData в JSON
    ↓
Запись .inclproj файла
```

## Формат проекта (.inclproj)

```json
{
  "version": 1,
  "name": "Проект А",
  "description": "Описание",
  "created": "2025-01-01T12:00:00",
  "modified": "2025-01-02T15:30:00",
  "wells": [
    {
      "name": "Скважина-1",
      "field": "Месторождение",
      "cluster": "Куст-1",
      "visible": true,
      "color": "#ff0000",
      "line_width": 2.0,
      "params": {
        "method": "minimum-curvature",
        "start_tvd": 0.0,
        "magnetic_declination": 5.5
      },
      "measurements": [
        {"md": 0.0, "inc": 0.0, "az": 0.0},
        {"md": 100.0, "inc": 5.5, "az": 45.0}
      ],
      "results": [
        {"md": 0.0, "tvd": 0.0, "north": 0.0, "east": 0.0}
      ]
    }
  ],
  "project_points": [
    {
      "name": "Пласт А",
      "plan_tvd": 1000.0,
      "plan_north": 50.0,
      "plan_east": 30.0,
      "radius": 25.0
    }
  ],
  "shot_points": [
    {
      "name": "ПВ-1",
      "x": 100.0,
      "y": 200.0,
      "z": 0.0
    }
  ],
  "view_settings": {
    "background_color": "#ffffff",
    "grid_step": 100.0,
    "show_grid": true,
    "show_axes": true
  }
}
```

## Логирование

Логгер с ротацией файлов:

```cpp
Logger::instance().setLogFile("~/.incline3d/logs/incline3d.log");
Logger::instance().setMaxFileSize(10 * 1024 * 1024);  // 10 МБ
Logger::instance().setMaxBackupCount(5);

LOG_INFO("Сообщение");
LOG_WARNING("Предупреждение: " + msg);
LOG_ERROR("Ошибка: " + error);
```

## Обработка ошибок

Все ошибки перехватываются и отображаются через `QMessageBox`:

```cpp
try {
    auto result = fileIO->loadWell(path);
    if (!result.success) {
        QMessageBox::critical(this, tr("Ошибка"), result.error_message);
        LOG_ERROR(result.error_message.toStdString());
    }
} catch (const std::exception& e) {
    QMessageBox::critical(this, tr("Критическая ошибка"), e.what());
    LOG_ERROR(std::string("Exception: ") + e.what());
}
```

## Тестирование

Модульные тесты на Qt Test:

```bash
ctest --test-dir build --output-on-failure
```

Тестируемые компоненты:
- `test_well_data` — структуры данных
- `test_angle_utils` — работа с углами
- `test_well_table_model` — Qt-модель скважин
- `test_project_manager` — управление проектом
- `test_process_runner` — интеграция с inclproc

## Расширение

### Добавление нового формата файла

1. Добавить парсер в `FileIO`:
   ```cpp
   WellData parseNewFormat(const QString& path);
   ```

2. Обновить фильтры:
   ```cpp
   static QString getOpenFileFilter() {
       return tr("...; Новый формат (*.new)");
   }
   ```

### Добавление нового вида визуализации

1. Создать класс в `views/`:
   ```cpp
   class NewView : public QWidget { ... };
   ```

2. Добавить вкладку в `MainWindow::createCentralWidget()`:
   ```cpp
   new_view_ = new NewView(this);
   central_tabs_->addTab(new_view_, tr("Новый вид"));
   ```

### Добавление новой команды inclproc

1. Определить структуру параметров в `incline_process_runner.h`
2. Реализовать метод `buildXxxArgs()` и `runXxx()`
3. Добавить диалог или пункт меню в UI

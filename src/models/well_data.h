#pragma once

#include <optional>
#include <string>
#include <vector>
#include <QColor>

namespace incline3d::models {

/// Единицы измерения углов для ввода/отображения
enum class AngleUnit {
    kDecimalDegrees,    ///< Десятичные градусы (по умолчанию)
    kDegreesMinutes     ///< Градусы и минуты (формат XX.YY где YY — минуты)
};

/// Тип азимута
enum class AzimuthType {
    kMagnetic,          ///< Магнитный азимут
    kTrue,              ///< Истинный азимут (географический)
    kGrid               ///< Сеточный азимут
};

/// Метод расчёта траектории
enum class CalculationMethod {
    kAverageAngle,
    kBalancedTangential,
    kMinimumCurvature,
    kRadiusOfCurvature,
    kRingArc
};

/// Точка измерения инклинометрии (исходные данные из массива ЗНАЧЕНИЯ)
struct MeasuredPoint {
    double measured_depth_m{0.0};        ///< Глубина по кабелю, м
    double inclination_deg{0.0};         ///< Зенитный угол, градусы
    std::optional<double> azimuth_deg;   ///< Магнитный азимут, градусы (может отсутствовать)
    std::optional<double> azimuth_true_deg; ///< Истинный азимут, градусы (может отсутствовать)
    AzimuthType azimuth_type{AzimuthType::kMagnetic}; ///< Тип используемого азимута
};

/// Результат расчёта для точки траектории
struct ProcessedPoint {
    double measured_depth_m{0.0};        ///< Глубина по стволу, м
    double inclination_deg{0.0};         ///< Угол наклона, градусы
    std::optional<double> azimuth_deg;   ///< Исходный азимут, градусы
    double applied_azimuth_deg{0.0};     ///< Приведённый азимут, градусы (истинный)
    double north_m{0.0};                 ///< Смещение на север, м
    double east_m{0.0};                  ///< Смещение на восток, м
    double tvd_m{0.0};                   ///< Вертикальная глубина, м
    std::optional<double> tvd_bgl_m;     ///< TVD ниже уровня земли, м
    std::optional<double> tvd_bml_m;     ///< TVD ниже уровня моря, м
    std::optional<double> absolute_elevation_m; ///< Абсолютная отметка, м
    double dogleg_angle_deg{0.0};        ///< Угол пространственного искривления, градусы
    double intensity_10m{0.0};           ///< Интенсивность на 10 м, град/10м
    double intensity_L{0.0};             ///< Интенсивность на интервал L, град/L
    double smoothed_intensity_10m{0.0};  ///< Сглаженная интенсивность на 10 м
    double smoothed_intensity_L{0.0};    ///< Сглаженная интенсивность на L
    double mistake_x{0.0};               ///< Ошибка по X, м
    double mistake_y{0.0};               ///< Ошибка по Y, м
    double mistake_z{0.0};               ///< Ошибка по Z, м
    double mistake_absg{0.0};            ///< Ошибка абсолютного смещения, м
    double mistake_intensity{0.0};       ///< Ошибка интенсивности, град/10м
};

/// Параметры расчёта траектории
struct CalculationParams {
    CalculationMethod method{CalculationMethod::kMinimumCurvature};
    double magnetic_declination_deg{0.0};   ///< Магнитное склонение, градусы
    double meridian_convergence_deg{0.0};   ///< Сближение меридианов, градусы
    double intensity_interval_m{30.0};      ///< Интервал для расчёта интенсивности L, м
    double min_inclination_for_xy_deg{0.0}; ///< Мин. угол для расчёта X/Y
    double vertical_limit_deg{3.0};         ///< Предел вертикального участка
    double error_depth_m{0.1};              ///< Погрешность глубины, м
    double error_inclination_deg{0.1};      ///< Погрешность угла, градусы
    double error_azimuth_deg{0.1};          ///< Погрешность азимута, градусы
    double intensity_threshold_deg{0.0};    ///< Порог предупреждения по интенсивности
    double delta_depth_warning_m{0.0};      ///< Порог предупреждения по шагу глубины
    double interpolation_step_m{0.0};       ///< Шаг интерполяции (0 = выкл)
    bool use_last_azimuth{true};            ///< Использовать последний азимут
    bool interpolate_missing_azimuths{true};///< Интерполировать пропущенные азимуты
    bool unwrap_azimuths{true};             ///< Разворачивать азимуты
    bool smooth_intensity{false};           ///< Сглаживать интенсивность
    bool sngf_mode{false};                  ///< Режим SNGF для азимутов
    double sngf_min_angle_deg{5.0};         ///< Мин. угол для SNGF-режима
    AzimuthType azimuth_type{AzimuthType::kMagnetic};

    // Параметры высотных отметок
    double kelly_bushing_elevation_m{0.0};  ///< Альтитуда устья, м
    double ground_elevation_m{0.0};         ///< Альтитуда земли, м
    double water_depth_m{0.0};              ///< Глубина воды (морские), м

    // Параметры контроля качества
    bool quality_check{false};
    double max_angle_deviation_deg{5.0};
    double max_azimuth_deviation_deg{10.0};
};

/// Метаданные скважины (соответствует полям ИНТЕРВАЛЫ_ИНКЛ в PrimeINCL)
struct WellMetadata {
    // Идентификация скважины
    std::string uwi;                        ///< Уникальный идентификатор (UWI)
    std::string well_name;                  ///< Название скважины
    std::string field_name;                 ///< Месторождение
    std::string area;                       ///< Площадь
    std::string well_pad;                   ///< Куст
    std::string region;                     ///< Регион
    std::string measurement_number;         ///< Номер измерения
    std::string file_name;                  ///< Имя файла

    // Прибор
    std::string device;                     ///< Тип прибора
    std::string device_number;              ///< Номер прибора
    std::string device_calibration_date;    ///< Дата поверки прибора

    // Интервал измерений
    double interval_start{0.0};             ///< Начало интервала, м
    double interval_end{0.0};               ///< Конец интервала, м

    // Магнитное склонение и высотные отметки
    double magnetic_declination{0.0};       ///< Магнитное склонение (или суммарная поправка), градусы
    double kelly_bushing{0.0};              ///< Альтитуда стола ротора, м
    double casing_shoe{0.0};                ///< Башмак кондуктора, м
    double ground_elevation{0.0};           ///< Альтитуда земли, м

    // Параметры скважины
    double d_casing{0.0};                   ///< Диаметр скважины, мм
    double d_collar{0.0};                   ///< Диаметр колонны, мм
    double current_depth{0.0};              ///< Текущий забой, м
    double project_depth{0.0};              ///< Проектный забой, м

    // Проектные параметры забоя
    double project_shift{0.0};              ///< Проектное смещение забоя, м
    double project_shift_error{0.0};        ///< Погрешность проектного смещения, м
    double project_azimuth{0.0};            ///< Проектный азимут забоя (истинный), градусы
    double project_azimuth_magnetic{0.0};   ///< Проектный азимут забоя (магнитный), градусы
    double tolerance_radius{0.0};           ///< Допустимый отход забоя (радиус допуска), м

    // Погрешности измерений
    double angle_error{0.1};                ///< Погрешность измерения угла, градусы
    double azimuth_error{0.1};              ///< Погрешность измерения азимута, градусы

    // Организационные данные
    std::string research_date;              ///< Дата исследования
    std::string conditions;                 ///< Условия исследования
    std::string research_type;              ///< Характер исследования (вид)
    std::string quality;                    ///< Качество измерения
    std::string lbt;                        ///< ЛБТ
    std::string ubt;                        ///< УБТ
    std::string customer_rep;               ///< Представитель заказчика
    std::string customer;                   ///< Заказчик
    std::string contractor;                 ///< Подрядчик
    std::string interpreter;                ///< Интерпретатор
    std::string party_chief;                ///< Начальник партии

    std::string comment;                    ///< Комментарий
};

/// Полные данные скважины (исходные и результаты)
struct WellData {
    WellMetadata metadata;
    std::vector<MeasuredPoint> measurements;
    std::vector<ProcessedPoint> results;
    CalculationParams params;

    // Сводные данные (заполняются после расчёта)
    double max_inclination_deg{0.0};        ///< Максимальный угол
    double max_intensity_10m{0.0};          ///< Максимальная интенсивность на 10 м
    double max_intensity_10m_depth{0.0};    ///< Глубина макс. интенсивности
    double max_intensity_L{0.0};            ///< Максимальная интенсивность на L
    double max_intensity_L_depth{0.0};      ///< Глубина макс. интенсивности на L
    double total_depth{0.0};                ///< Общая глубина, м
    double horizontal_displacement{0.0};    ///< Горизонтальное смещение забоя, м

    // Визуальные настройки для отображения
    QColor display_color{Qt::blue};         ///< Цвет траектории
    bool visible{true};                     ///< Видимость в визуализации
    int line_width{2};                      ///< Толщина линии

    // Путь к исходному файлу
    std::string source_file_path;
    std::string source_format;              ///< "ws", "csv", "las", "zak"

    // Флаг модификации (для отслеживания несохранённых изменений)
    bool modified{false};
};

/// Конвертация названия метода в строку
std::string method_to_string(CalculationMethod method);

/// Конвертация строки в метод
CalculationMethod string_to_method(const std::string& str);

/// Конвертация типа азимута в строку
std::string azimuth_type_to_string(AzimuthType type);

/// Конвертация строки в тип азимута
AzimuthType string_to_azimuth_type(const std::string& str);

}  // namespace incline3d::models

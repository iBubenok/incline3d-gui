#include "utils/angle_utils.h"

#include <QRegularExpression>

namespace incline3d::utils {

bool parse_angle(const QString& str, double& result) {
    QString s = str.trimmed();
    if (s.isEmpty()) {
        return false;
    }

    // Попытка парсинга как простого числа
    bool ok = false;
    result = s.toDouble(&ok);
    if (ok) {
        return true;
    }

    // Формат: XX°YY'ZZ" (градусы, минуты, секунды)
    QRegularExpression re_dms(R"((-?\d+)[°]\s*(\d+)['']\s*(\d+(?:\.\d+)?)[""\"]\s*)");
    QRegularExpressionMatch match = re_dms.match(s);
    if (match.hasMatch()) {
        double deg = match.captured(1).toDouble();
        double min = match.captured(2).toDouble();
        double sec = match.captured(3).toDouble();
        result = deg + min / 60.0 + sec / 3600.0;
        return true;
    }

    // Формат: XX°YY' (градусы, минуты)
    QRegularExpression re_dm(R"((-?\d+)[°]\s*(\d+(?:\.\d+)?)['']\s*)");
    match = re_dm.match(s);
    if (match.hasMatch()) {
        double deg = match.captured(1).toDouble();
        double min = match.captured(2).toDouble();
        result = deg + min / 60.0;
        return true;
    }

    // Формат: XX° (только градусы)
    QRegularExpression re_d(R"((-?\d+(?:\.\d+)?)[°]\s*)");
    match = re_d.match(s);
    if (match.hasMatch()) {
        result = match.captured(1).toDouble();
        return true;
    }

    // Формат XX.YY как градусы.минуты (если явно указано)
    // Проверяем, похоже ли значение на формат градусы.минуты
    // (дробная часть < 0.60)
    double test_val = 0;
    ok = false;
    test_val = s.replace(',', '.').toDouble(&ok);
    if (ok) {
        double frac = test_val - std::floor(test_val);
        // Если дробная часть выглядит как минуты (0-59/100)
        if (frac < 0.60) {
            // Это может быть формат градусы.минуты
            // Но по умолчанию считаем десятичными градусами
            result = test_val;
            return true;
        }
        result = test_val;
        return true;
    }

    return false;
}

}  // namespace incline3d::utils

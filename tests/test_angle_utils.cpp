#include <QtTest>
#include <cmath>

#include "utils/angle_utils.h"

using namespace incline3d::utils;

class TestAngleUtils : public QObject {
    Q_OBJECT

private slots:
    void testDegToRad();
    void testRadToDeg();
    void testDegFromDegmin();
    void testDegToDegmin();
    void testNormalizeAngle360();
    void testNormalizeAngle180();
    void testFormatAngleDecimal();
    void testFormatAngleDegmin();
    void testFormatAngleDms();
    void testParseAngle();
    void testCalculateAzimuth();
    void testCalculateDistance();
};

void TestAngleUtils::testDegToRad() {
    QCOMPARE(deg_to_rad(0.0), 0.0);
    QVERIFY(std::abs(deg_to_rad(90.0) - PI / 2.0) < 1e-10);
    QVERIFY(std::abs(deg_to_rad(180.0) - PI) < 1e-10);
    QVERIFY(std::abs(deg_to_rad(360.0) - 2 * PI) < 1e-10);
    QVERIFY(std::abs(deg_to_rad(-90.0) + PI / 2.0) < 1e-10);
}

void TestAngleUtils::testRadToDeg() {
    QCOMPARE(rad_to_deg(0.0), 0.0);
    QVERIFY(std::abs(rad_to_deg(PI / 2.0) - 90.0) < 1e-10);
    QVERIFY(std::abs(rad_to_deg(PI) - 180.0) < 1e-10);
    QVERIFY(std::abs(rad_to_deg(2 * PI) - 360.0) < 1e-10);
}

void TestAngleUtils::testDegFromDegmin() {
    // 45°30' = 45.5°
    QVERIFY(std::abs(deg_from_degmin(45.30) - 45.5) < 1e-10);

    // 90°00' = 90°
    QVERIFY(std::abs(deg_from_degmin(90.00) - 90.0) < 1e-10);

    // 10°15' = 10.25°
    QVERIFY(std::abs(deg_from_degmin(10.15) - 10.25) < 1e-10);

    // 0°30' = 0.5°
    QVERIFY(std::abs(deg_from_degmin(0.30) - 0.5) < 1e-10);
}

void TestAngleUtils::testDegToDegmin() {
    // 45.5° = 45°30'
    QVERIFY(std::abs(deg_to_degmin(45.5) - 45.30) < 1e-10);

    // 90.0° = 90°00'
    QVERIFY(std::abs(deg_to_degmin(90.0) - 90.00) < 1e-10);

    // 10.25° = 10°15'
    QVERIFY(std::abs(deg_to_degmin(10.25) - 10.15) < 1e-10);

    // Обратное преобразование
    double original = 123.456;
    double degmin = deg_to_degmin(original);
    double restored = deg_from_degmin(degmin);
    QVERIFY(std::abs(original - restored) < 1e-6);
}

void TestAngleUtils::testNormalizeAngle360() {
    // В диапазоне [0, 360)
    QVERIFY(std::abs(normalize_angle_360(0.0) - 0.0) < 1e-10);
    QVERIFY(std::abs(normalize_angle_360(90.0) - 90.0) < 1e-10);
    QVERIFY(std::abs(normalize_angle_360(360.0) - 0.0) < 1e-10);
    QVERIFY(std::abs(normalize_angle_360(450.0) - 90.0) < 1e-10);
    QVERIFY(std::abs(normalize_angle_360(-90.0) - 270.0) < 1e-10);
    QVERIFY(std::abs(normalize_angle_360(-360.0) - 0.0) < 1e-10);
}

void TestAngleUtils::testNormalizeAngle180() {
    // В диапазоне [-180, 180)
    QVERIFY(std::abs(normalize_angle_180(0.0) - 0.0) < 1e-10);
    QVERIFY(std::abs(normalize_angle_180(90.0) - 90.0) < 1e-10);
    QVERIFY(std::abs(normalize_angle_180(180.0) + 180.0) < 1e-10);
    QVERIFY(std::abs(normalize_angle_180(270.0) + 90.0) < 1e-10);
    QVERIFY(std::abs(normalize_angle_180(-90.0) + 90.0) < 1e-10);
}

void TestAngleUtils::testFormatAngleDecimal() {
    QString result = format_angle_decimal(45.5, 2);
    QCOMPARE(result, QString::fromUtf8("45.50°"));

    result = format_angle_decimal(0.0, 1);
    QCOMPARE(result, QString::fromUtf8("0.0°"));
}

void TestAngleUtils::testFormatAngleDegmin() {
    QString result = format_angle_degmin(45.5);
    QVERIFY(result.contains("45"));
    QVERIFY(result.contains("30"));

    result = format_angle_degmin(90.0);
    QVERIFY(result.contains("90"));
}

void TestAngleUtils::testFormatAngleDms() {
    QString result = format_angle_dms(45.5);
    // 45.5° = 45°30'0"
    QVERIFY(result.contains("45"));
    QVERIFY(result.contains("30"));
}

void TestAngleUtils::testParseAngle() {
    double val;
    bool ok;

    // Простое число
    ok = parse_angle("45.5", val);
    QVERIFY(ok);
    QVERIFY(std::abs(val - 45.5) < 1e-10);

    // С градусом
    ok = parse_angle(QString::fromUtf8("90°"), val);
    QVERIFY(ok);
    QVERIFY(std::abs(val - 90.0) < 1e-10);

    // Невалидная строка
    ok = parse_angle("abc", val);
    QVERIFY(!ok);

    // Пустая строка
    ok = parse_angle("", val);
    QVERIFY(!ok);
}

void TestAngleUtils::testCalculateAzimuth() {
    // Север (dy=1, dx=0) -> азимут 0
    double azim = calculate_azimuth(0.0, 1.0);
    QVERIFY(std::abs(azim - 0.0) < 1e-10);

    // Восток (dy=0, dx=1) -> азимут 90
    azim = calculate_azimuth(1.0, 0.0);
    QVERIFY(std::abs(azim - 90.0) < 1e-10);

    // Юг (dy=-1, dx=0) -> азимут 180
    azim = calculate_azimuth(0.0, -1.0);
    QVERIFY(std::abs(azim - 180.0) < 1e-10);

    // Запад (dy=0, dx=-1) -> азимут 270
    azim = calculate_azimuth(-1.0, 0.0);
    QVERIFY(std::abs(azim - 270.0) < 1e-10);
}

void TestAngleUtils::testCalculateDistance() {
    // Пифагорова тройка: 3, 4, 5
    double dist = calculate_distance(3.0, 4.0);
    QVERIFY(std::abs(dist - 5.0) < 1e-10);

    // Нулевое расстояние
    dist = calculate_distance(0.0, 0.0);
    QCOMPARE(dist, 0.0);

    // 3D расстояние
    double dist3d = calculate_distance_3d(1.0, 2.0, 2.0);
    QVERIFY(std::abs(dist3d - 3.0) < 1e-10);
}

QTEST_MAIN(TestAngleUtils)
#include "test_angle_utils.moc"

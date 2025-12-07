#include <QtTest>

#include "models/well_data.h"

using namespace incline3d::models;

class TestWellData : public QObject {
    Q_OBJECT

private slots:
    void testMeasuredPointDefaults();
    void testProcessedPointDefaults();
    void testCalculationParamsDefaults();
    void testWellMetadataDefaults();
    void testWellDataDefaults();
    void testCalculationMethodValues();
    void testAzimuthTypeValues();
    void testMethodToString();
    void testStringToMethod();
};

void TestWellData::testMeasuredPointDefaults() {
    MeasuredPoint pt;

    QCOMPARE(pt.measured_depth_m, 0.0);
    QCOMPARE(pt.inclination_deg, 0.0);
    QVERIFY(!pt.azimuth_deg.has_value());
    QCOMPARE(pt.azimuth_type, AzimuthType::kMagnetic);
}

void TestWellData::testProcessedPointDefaults() {
    ProcessedPoint pt;

    QCOMPARE(pt.measured_depth_m, 0.0);
    QCOMPARE(pt.inclination_deg, 0.0);
    QCOMPARE(pt.tvd_m, 0.0);
    QCOMPARE(pt.north_m, 0.0);
    QCOMPARE(pt.east_m, 0.0);
    QCOMPARE(pt.dogleg_angle_deg, 0.0);
    QCOMPARE(pt.intensity_10m, 0.0);
    QCOMPARE(pt.intensity_L, 0.0);
    QCOMPARE(pt.mistake_x, 0.0);
    QCOMPARE(pt.mistake_y, 0.0);
    QCOMPARE(pt.mistake_z, 0.0);
}

void TestWellData::testCalculationParamsDefaults() {
    CalculationParams params;

    QCOMPARE(params.method, CalculationMethod::kMinimumCurvature);
    QCOMPARE(params.magnetic_declination_deg, 0.0);
    QCOMPARE(params.meridian_convergence_deg, 0.0);
    QCOMPARE(params.intensity_interval_m, 30.0);
    QCOMPARE(params.vertical_limit_deg, 3.0);
    QCOMPARE(params.error_depth_m, 0.1);
    QCOMPARE(params.error_inclination_deg, 0.1);
    QCOMPARE(params.error_azimuth_deg, 0.1);
    QVERIFY(params.use_last_azimuth);
    QVERIFY(params.interpolate_missing_azimuths);
    QVERIFY(!params.smooth_intensity);
    QVERIFY(!params.sngf_mode);
}

void TestWellData::testWellMetadataDefaults() {
    WellMetadata meta;

    QVERIFY(meta.well_name.empty());
    QVERIFY(meta.field_name.empty());
    QVERIFY(meta.well_pad.empty());
    QVERIFY(meta.area.empty());
    QVERIFY(meta.region.empty());
    QVERIFY(meta.uwi.empty());
    QVERIFY(meta.comment.empty());
    QVERIFY(meta.measurement_number.empty());
}

void TestWellData::testWellDataDefaults() {
    WellData well;

    QVERIFY(well.measurements.empty());
    QVERIFY(well.results.empty());
    QVERIFY(well.visible);
    QVERIFY(!well.modified);
    QCOMPARE(well.line_width, 2);
    QVERIFY(well.source_file_path.empty());
    QCOMPARE(well.max_inclination_deg, 0.0);
    QCOMPARE(well.total_depth, 0.0);
    QCOMPARE(well.horizontal_displacement, 0.0);
}

void TestWellData::testCalculationMethodValues() {
    // Проверка корректности enum значений
    QCOMPARE(static_cast<int>(CalculationMethod::kAverageAngle), 0);
    QCOMPARE(static_cast<int>(CalculationMethod::kBalancedTangential), 1);
    QCOMPARE(static_cast<int>(CalculationMethod::kMinimumCurvature), 2);
    QCOMPARE(static_cast<int>(CalculationMethod::kRadiusOfCurvature), 3);
    QCOMPARE(static_cast<int>(CalculationMethod::kRingArc), 4);
}

void TestWellData::testAzimuthTypeValues() {
    QCOMPARE(static_cast<int>(AzimuthType::kMagnetic), 0);
    QCOMPARE(static_cast<int>(AzimuthType::kTrue), 1);
    QCOMPARE(static_cast<int>(AzimuthType::kGrid), 2);
}

void TestWellData::testMethodToString() {
    // Краткие имена методов (используются в CLI)
    QCOMPARE(method_to_string(CalculationMethod::kMinimumCurvature),
             std::string("mincurv"));
    QCOMPARE(method_to_string(CalculationMethod::kBalancedTangential),
             std::string("balanced"));
    QCOMPARE(method_to_string(CalculationMethod::kAverageAngle),
             std::string("average"));
    QCOMPARE(method_to_string(CalculationMethod::kRadiusOfCurvature),
             std::string("radiuscurv"));
    QCOMPARE(method_to_string(CalculationMethod::kRingArc),
             std::string("ringarc"));
}

void TestWellData::testStringToMethod() {
    QCOMPARE(string_to_method("minimum-curvature"),
             CalculationMethod::kMinimumCurvature);
    QCOMPARE(string_to_method("balanced-tangential"),
             CalculationMethod::kBalancedTangential);
    QCOMPARE(string_to_method("average-angle"),
             CalculationMethod::kAverageAngle);

    // Неизвестная строка должна вернуть метод по умолчанию
    QCOMPARE(string_to_method("unknown"),
             CalculationMethod::kMinimumCurvature);
}

QTEST_MAIN(TestWellData)
#include "test_well_data.moc"

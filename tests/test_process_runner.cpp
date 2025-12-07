#include <QtTest>
#include <QSignalSpy>

#include "core/incline_process_runner.h"
#include "models/well_data.h"

using namespace incline3d::core;
using namespace incline3d::models;

class TestProcessRunner : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testDefaultInclprocPath();
    void testSetInclprocPath();
    void testIsRunning();
    void testProcessResultDefaults();

private:
    InclineProcessRunner* runner_{nullptr};
};

void TestProcessRunner::initTestCase() {
    runner_ = new InclineProcessRunner();
}

void TestProcessRunner::cleanupTestCase() {
    delete runner_;
}

void TestProcessRunner::testDefaultInclprocPath() {
    // Путь по умолчанию из INCLPROC_DEFAULT_PATH или пустой
    QString path = runner_->inclprocPath();
    // В тестовом окружении путь может быть "inclproc" или путь из define
    QVERIFY(!path.isEmpty() || path == "inclproc");
}

void TestProcessRunner::testSetInclprocPath() {
    runner_->setInclprocPath("/usr/local/bin/inclproc");
    QCOMPARE(runner_->inclprocPath(), QString("/usr/local/bin/inclproc"));

    // Вернём обратно
    runner_->setInclprocPath("inclproc");
}

void TestProcessRunner::testIsRunning() {
    // Изначально процесс не запущен
    QVERIFY(!runner_->isRunning());
}

void TestProcessRunner::testProcessResultDefaults() {
    ProcessResult result;

    QVERIFY(!result.success);
    QCOMPARE(result.exit_code, 0);
    QVERIFY(result.stdout_output.isEmpty());
    QVERIFY(result.stderr_output.isEmpty());
    QVERIFY(result.error_message.isEmpty());
    QVERIFY(!result.min_distance.has_value());
    QVERIFY(!result.horizontal_offset.has_value());
}

QTEST_MAIN(TestProcessRunner)
#include "test_process_runner.moc"

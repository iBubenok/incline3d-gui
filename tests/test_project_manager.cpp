#include <QtTest>
#include <QTemporaryDir>
#include <QSignalSpy>

#include "core/project_manager.h"
#include "models/well_data.h"
#include "models/project_point.h"
#include "models/shot_point.h"

using namespace incline3d::core;
using namespace incline3d::models;

class TestProjectManager : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testNewProject();
    void testAddWell();
    void testRemoveWell();
    void testDirtyState();
    void testProjectFileFilter();
    void testSignals();
    void testProjectData();
    void testViewSettings();

private:
    ProjectManager* manager_{nullptr};
    QTemporaryDir* temp_dir_{nullptr};
};

void TestProjectManager::initTestCase() {
    manager_ = new ProjectManager();
    temp_dir_ = new QTemporaryDir();
    QVERIFY(temp_dir_->isValid());
}

void TestProjectManager::cleanupTestCase() {
    delete manager_;
    delete temp_dir_;
}

void TestProjectManager::testNewProject() {
    manager_->newProject();

    QVERIFY(!manager_->isDirty());
    QVERIFY(manager_->projectFilePath().isEmpty());
    QVERIFY(manager_->wells().empty());

    const auto& data = manager_->projectData();
    QCOMPARE(data.version, 1);
}

void TestProjectManager::testAddWell() {
    manager_->newProject();

    auto well = std::make_shared<WellData>();
    well->metadata.well_name = "Тестовая скважина";

    manager_->addWell(well);

    QCOMPARE(manager_->wells().size(), static_cast<size_t>(1));
    QVERIFY(manager_->isDirty());
}

void TestProjectManager::testRemoveWell() {
    manager_->newProject();

    auto well1 = std::make_shared<WellData>();
    well1->metadata.well_name = "Скважина 1";
    auto well2 = std::make_shared<WellData>();
    well2->metadata.well_name = "Скважина 2";

    manager_->addWell(well1);
    manager_->addWell(well2);
    QCOMPARE(manager_->wells().size(), static_cast<size_t>(2));

    manager_->removeWell(0);
    QCOMPARE(manager_->wells().size(), static_cast<size_t>(1));
    QCOMPARE(manager_->wells().at(0)->metadata.well_name, std::string("Скважина 2"));
}

void TestProjectManager::testDirtyState() {
    manager_->newProject();
    QVERIFY(!manager_->isDirty());

    manager_->setDirty(true);
    QVERIFY(manager_->isDirty());

    manager_->setDirty(false);
    QVERIFY(!manager_->isDirty());

    // Добавление скважины устанавливает dirty
    auto well = std::make_shared<WellData>();
    manager_->addWell(well);
    QVERIFY(manager_->isDirty());
}

void TestProjectManager::testProjectFileFilter() {
    QString filter = ProjectManager::getProjectFileFilter();
    QVERIFY(filter.contains("*.inclproj"));
}

void TestProjectManager::testSignals() {
    QSignalSpy createdSpy(manager_, &ProjectManager::projectCreated);
    QSignalSpy dirtySpy(manager_, &ProjectManager::dirtyChanged);
    QSignalSpy wellsSpy(manager_, &ProjectManager::wellsChanged);

    manager_->newProject();
    QCOMPARE(createdSpy.count(), 1);

    auto well = std::make_shared<WellData>();
    manager_->addWell(well);
    // wellsChanged может вызываться несколько раз при добавлении
    QVERIFY(wellsSpy.count() >= 1);
    // dirtyChanged может вызываться несколько раз
    QVERIFY(dirtySpy.count() >= 1);
}

void TestProjectManager::testProjectData() {
    manager_->newProject();

    ProjectData& data = manager_->projectData();
    QCOMPARE(data.version, 1);
    QVERIFY(data.name.isEmpty());
    QVERIFY(data.project_points.empty());
    QVERIFY(data.shot_points.empty());
    QVERIFY(data.well_entries.empty());
}

void TestProjectManager::testViewSettings() {
    ViewSettings settings;

    // Значения по умолчанию для 3D вида
    QCOMPARE(settings.rotation_x, 30.0);
    QCOMPARE(settings.rotation_y, -45.0);
    QCOMPARE(settings.rotation_z, 0.0);
    QCOMPARE(settings.scale, 1.0);

    // Значения по умолчанию для плана
    QCOMPARE(settings.plan_scale, 1.0);
    QCOMPARE(settings.plan_center_x, 0.0);
    QCOMPARE(settings.plan_center_y, 0.0);

    // Значения по умолчанию для вертикальной проекции
    QCOMPARE(settings.vertical_azimuth, 0.0);
}

QTEST_MAIN(TestProjectManager)
#include "test_project_manager.moc"

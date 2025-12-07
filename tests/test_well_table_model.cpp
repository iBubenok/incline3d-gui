#include <QtTest>
#include <QSignalSpy>

#include "models/well_table_model.h"

using namespace incline3d::models;

class TestWellTableModel : public QObject {
    Q_OBJECT

private slots:
    void initTestCase();
    void cleanupTestCase();

    void testInitialState();
    void testAddWell();
    void testRemoveWell();
    void testClear();
    void testWellAt();
    void testRowCount();
    void testColumnCount();
    void testHeaderData();
    void testFindWellByName();
    void testVisibilityChange();

private:
    WellTableModel* model_{nullptr};
};

void TestWellTableModel::initTestCase() {
    model_ = new WellTableModel();
}

void TestWellTableModel::cleanupTestCase() {
    delete model_;
}

void TestWellTableModel::testInitialState() {
    WellTableModel empty_model;
    QCOMPARE(empty_model.wellCount(), 0);
    QCOMPARE(empty_model.rowCount(), 0);
}

void TestWellTableModel::testAddWell() {
    model_->clear();

    auto well = std::make_shared<WellData>();
    well->metadata.well_name = "Тестовая скважина 1";
    well->metadata.field_name = "Тестовое месторождение";

    QSignalSpy spy(model_, &QAbstractItemModel::rowsInserted);

    model_->addWell(well);

    QCOMPARE(model_->wellCount(), 1);
    QCOMPARE(spy.count(), 1);

    // Добавим ещё одну
    auto well2 = std::make_shared<WellData>();
    well2->metadata.well_name = "Тестовая скважина 2";
    model_->addWell(well2);

    QCOMPARE(model_->wellCount(), 2);
}

void TestWellTableModel::testRemoveWell() {
    model_->clear();

    auto well1 = std::make_shared<WellData>();
    well1->metadata.well_name = "Скважина 1";
    auto well2 = std::make_shared<WellData>();
    well2->metadata.well_name = "Скважина 2";

    model_->addWell(well1);
    model_->addWell(well2);
    QCOMPARE(model_->wellCount(), 2);

    QSignalSpy spy(model_, &QAbstractItemModel::rowsRemoved);

    model_->removeWell(0);

    QCOMPARE(model_->wellCount(), 1);
    QCOMPARE(spy.count(), 1);

    // Проверяем, что осталась вторая скважина
    auto remaining = model_->wellAt(0);
    QVERIFY(remaining != nullptr);
    QCOMPARE(remaining->metadata.well_name, std::string("Скважина 2"));
}

void TestWellTableModel::testClear() {
    model_->clear();

    auto well = std::make_shared<WellData>();
    model_->addWell(well);
    model_->addWell(well);
    model_->addWell(well);

    QCOMPARE(model_->wellCount(), 3);

    model_->clear();

    QCOMPARE(model_->wellCount(), 0);
}

void TestWellTableModel::testWellAt() {
    model_->clear();

    auto well = std::make_shared<WellData>();
    well->metadata.well_name = "Тест";
    model_->addWell(well);

    auto retrieved = model_->wellAt(0);
    QVERIFY(retrieved != nullptr);
    QCOMPARE(retrieved->metadata.well_name, std::string("Тест"));

    // Невалидный индекс
    auto invalid = model_->wellAt(-1);
    QVERIFY(invalid == nullptr);

    invalid = model_->wellAt(100);
    QVERIFY(invalid == nullptr);
}

void TestWellTableModel::testRowCount() {
    model_->clear();
    QCOMPARE(model_->rowCount(), 0);

    auto well = std::make_shared<WellData>();
    model_->addWell(well);
    QCOMPARE(model_->rowCount(), 1);

    model_->addWell(well);
    model_->addWell(well);
    QCOMPARE(model_->rowCount(), 3);
}

void TestWellTableModel::testColumnCount() {
    // Должно быть kColumnCount колонок
    QCOMPARE(model_->columnCount(), static_cast<int>(WellTableModel::kColumnCount));
}

void TestWellTableModel::testHeaderData() {
    // Проверка заголовков колонок
    QVariant header0 = model_->headerData(WellTableModel::kColumnVisible, Qt::Horizontal);
    QVariant header1 = model_->headerData(WellTableModel::kColumnName, Qt::Horizontal);
    QVariant header2 = model_->headerData(WellTableModel::kColumnField, Qt::Horizontal);

    // Заголовок видимости обычно пустой
    QVERIFY(header1.toString().contains(QString::fromUtf8("Скважина")) ||
            header1.toString().contains("Well"));
}

void TestWellTableModel::testFindWellByName() {
    model_->clear();

    auto well1 = std::make_shared<WellData>();
    well1->metadata.well_name = "Скважина-А";
    auto well2 = std::make_shared<WellData>();
    well2->metadata.well_name = "Скважина-Б";

    model_->addWell(well1);
    model_->addWell(well2);

    QCOMPARE(model_->findWellByName("Скважина-А"), 0);
    QCOMPARE(model_->findWellByName("Скважина-Б"), 1);
    QCOMPARE(model_->findWellByName("Несуществующая"), -1);
}

void TestWellTableModel::testVisibilityChange() {
    model_->clear();

    auto well = std::make_shared<WellData>();
    well->visible = true;
    model_->addWell(well);

    QSignalSpy visibilitySpy(model_, &WellTableModel::wellVisibilityChanged);

    QModelIndex idx = model_->index(0, WellTableModel::kColumnVisible);

    // Изменение видимости
    bool result = model_->setData(idx, Qt::Unchecked, Qt::CheckStateRole);
    QVERIFY(result);
    QVERIFY(!well->visible);
    QCOMPARE(visibilitySpy.count(), 1);

    // Обратно
    result = model_->setData(idx, Qt::Checked, Qt::CheckStateRole);
    QVERIFY(result);
    QVERIFY(well->visible);
}

QTEST_MAIN(TestWellTableModel)
#include "test_well_table_model.moc"

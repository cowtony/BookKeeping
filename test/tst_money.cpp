#include <QtTest>

// #include "../app/Book/money.h"

class TestMoney : public QObject
{
    Q_OBJECT

public:
    TestMoney();
    ~TestMoney();

private slots:
    void initTestCase();
    void cleanupTestCase();
    void test_case1();

};

TestMoney::TestMoney()
{

}

TestMoney::~TestMoney()
{

}

void TestMoney::initTestCase()
{

}

void TestMoney::cleanupTestCase()
{

}

void TestMoney::test_case1()
{

}

QTEST_APPLESS_MAIN(TestMoney)

#include "tst_money.moc"

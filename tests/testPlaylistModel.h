#include "playlistModel.h"
#include <QtTest/QtTest>

class TestPlaylistModel : public QObject
{
    Q_OBJECT

    NPlaylistModel *m_model;

    void createRows(size_t count)
    {
        for (size_t i = 0; i < count; ++i) {
            NPlaylistModel::DataItem modelItem;
            modelItem.text = QString("%1").arg(i);
            modelItem.filePath = QString("/path/%1").arg(i);
            m_model->append(modelItem);
        }
    }

private slots:
    void init() { m_model = new NPlaylistModel; }

    void cleanup()
    {
        delete m_model;
        m_model = nullptr;
    }

    void testSelectAndDeselect()
    {
        createRows(10);

        QList<size_t> rowsToSelect = {5, 3, 0, 2}; // unsorted
        for (size_t i : rowsToSelect) {
            m_model->setData(i, NPlaylistModel::IsSelectedRole, true);
        }

        QList<size_t> selectedRowsSorted = rowsToSelect;
        std::sort(selectedRowsSorted.begin(), selectedRowsSorted.end());
        QCOMPARE(m_model->selectedRows(), selectedRowsSorted);

        m_model->setData(NPlaylistModel::IsSelectedRole, false);
        QCOMPARE(m_model->selectedRows(), {});
    }

    void testFocusAbsolute()
    {
        createRows(10);

        // nothing focused by default:
        QCOMPARE(m_model->focusedRow(), -1);

        m_model->setFocusedRow(2);
        QCOMPARE(m_model->focusedRow(), 2);
        m_model->setFocusedRow(5);
        QCOMPARE(m_model->focusedRow(), 5);

        // invalid row does not change focus:
        m_model->setFocusedRow(-1);
        QCOMPARE(m_model->focusedRow(), 5);
        m_model->setFocusedRow(-100);
        QCOMPARE(m_model->focusedRow(), 5);
        m_model->setFocusedRow(100);
        QCOMPARE(m_model->focusedRow(), 5);
    }

    void testFocusRelative()
    {
        createRows(5);

        // nothing focused by default:
        QCOMPARE(m_model->focusedRow(), -1);

        m_model->setFocusedRow(2);
        QCOMPARE(m_model->focusedRow(), 2);
        m_model->setFocusedRowRelative(-1);
        QCOMPARE(m_model->focusedRow(), 1);
        m_model->setFocusedRowRelative(-1);
        QCOMPARE(m_model->focusedRow(), 0);
        m_model->setFocusedRowRelative(-1);
        QCOMPARE(m_model->focusedRow(), 0);

        m_model->setFocusedRow(2);
        QCOMPARE(m_model->focusedRow(), 2);
        m_model->setFocusedRowRelative(+1);
        QCOMPARE(m_model->focusedRow(), 3);
        m_model->setFocusedRowRelative(+1);
        QCOMPARE(m_model->focusedRow(), 4);
        m_model->setFocusedRowRelative(+1);
        QCOMPARE(m_model->focusedRow(), 4);

        // invalid offset is bound to model size:

        m_model->setFocusedRow(2);
        QCOMPARE(m_model->focusedRow(), 2);
        m_model->setFocusedRowRelative(-100);
        QCOMPARE(m_model->focusedRow(), 0);

        m_model->setFocusedRow(2);
        QCOMPARE(m_model->focusedRow(), 2);
        m_model->setFocusedRowRelative(+100);
        QCOMPARE(m_model->focusedRow(), m_model->size() - 1);
    }

    void testRemove_data()
    {
        QTest::addColumn<int>("countToCreate");
        QTest::addColumn<QList<size_t>>("rowsToRemove");
        QTest::addColumn<QList<size_t>>("expectRowsAfter");

        QTest::newRow("non-continuous, unsorted")
            << 10 << QList<size_t>{5, 3, 1, 7} << QList<size_t>{0, 2, 4, 6, 8, 9};
        QTest::newRow("continuous, sorted, remove from beginning")
            << 10 << QList<size_t>{0, 1, 2, 3} << QList<size_t>{4, 5, 6, 7, 8, 9};
        QTest::newRow("continuous, sorted, remove from end")
            << 10 << QList<size_t>{6, 7, 8, 9} << QList<size_t>{0, 1, 2, 3, 4, 5};
        QTest::newRow("remove all")
            << 10 << QList<size_t>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9} << QList<size_t>{};
    }

    void testRemove()
    {
        QFETCH(int, countToCreate);
        QFETCH(QList<size_t>, rowsToRemove);
        QFETCH(QList<size_t>, expectRowsAfter);

        createRows(countToCreate);

        m_model->remove(rowsToRemove);

        QStringList expectSongsAfter;
        for (size_t i : expectRowsAfter) {
            expectSongsAfter << QString("%1").arg(i);
        }
        QStringList actualSongsAfter;
        for (size_t i = 0; i < m_model->size(); ++i) {
            actualSongsAfter << m_model->data(i, NPlaylistModel::TextRole).toString();
        }
        QCOMPARE(expectSongsAfter, actualSongsAfter);
    }

    void testMove_data()
    {
        QTest::addColumn<int>("countToCreate");
        QTest::addColumn<QList<size_t>>("rowsToMove");
        QTest::addColumn<int>("beforeRow");
        QTest::addColumn<QList<size_t>>("expectRowsAfter");

        QTest::newRow("single, move to before first")
            << 10 << QList<size_t>{5} << 0 << QList<size_t>{5, 0, 1, 2, 3, 4, 6, 7, 8, 9};
        QTest::newRow("single, move to after first")
            << 10 << QList<size_t>{5} << 1 << QList<size_t>{0, 5, 1, 2, 3, 4, 6, 7, 8, 9};
        QTest::newRow("single, swap with previous")
            << 10 << QList<size_t>{5} << 4 << QList<size_t>{0, 1, 2, 3, 5, 4, 6, 7, 8, 9};
        QTest::newRow("single, swap with next")
            << 10 << QList<size_t>{5} << 6 << QList<size_t>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        QTest::newRow("single, to same place")
            << 10 << QList<size_t>{5} << 5 << QList<size_t>{0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
        QTest::newRow("single, move to before last")
            << 10 << QList<size_t>{5} << 9 << QList<size_t>{0, 1, 2, 3, 4, 6, 7, 8, 5, 9};
        QTest::newRow("single, move to after last")
            << 10 << QList<size_t>{5} << 10 << QList<size_t>{0, 1, 2, 3, 4, 6, 7, 8, 9, 5};

        QTest::newRow("multiple, move to to before first")
            << 10 << QList<size_t>{3, 5, 7} << 0 << QList<size_t>{3, 5, 7, 0, 1, 2, 4, 6, 8, 9};
        QTest::newRow("multiple, move to after first")
            << 10 << QList<size_t>{3, 5, 7} << 1 << QList<size_t>{0, 3, 5, 7, 1, 2, 4, 6, 8, 9};

        QTest::newRow("multiple, move to before last")
            << 10 << QList<size_t>{3, 5, 7} << 9 << QList<size_t>{0, 1, 2, 4, 6, 8, 3, 5, 7, 9};
        QTest::newRow("multiple, move to after last")
            << 10 << QList<size_t>{3, 5, 7} << 10 << QList<size_t>{0, 1, 2, 4, 6, 8, 9, 3, 5, 7};

        QTest::newRow("multiple, move to before selection")
            << 10 << QList<size_t>{3, 5, 7} << 3 << QList<size_t>{0, 1, 2, 3, 5, 7, 4, 6, 8, 9};
        //QTest::newRow("multiple, move to middle of selection")
        //    << 10 << QList<size_t>{3, 5, 7} << 5 << QList<size_t>{0, 1, 2, 4, 3, 5, 7, 6, 8, 9};
        QTest::newRow("multiple, move to after selection")
            << 10 << QList<size_t>{3, 5, 7} << 7 << QList<size_t>{0, 1, 2, 4, 6, 3, 5, 7, 8, 9};
    }

    void testMove()
    {
        QFETCH(int, countToCreate);
        QFETCH(QList<size_t>, rowsToMove);
        QFETCH(int, beforeRow);
        QFETCH(QList<size_t>, expectRowsAfter);

        createRows(countToCreate);

        m_model->move(rowsToMove, beforeRow);

        QStringList expectSongsAfter;
        for (size_t i : expectRowsAfter) {
            expectSongsAfter << QString("%1").arg(i);
        }
        QStringList actualSongsAfter;
        for (size_t i = 0; i < m_model->size(); ++i) {
            actualSongsAfter << m_model->data(i, NPlaylistModel::TextRole).toString();
        }
        QCOMPARE(expectSongsAfter, actualSongsAfter);
    }
};

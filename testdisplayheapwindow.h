#ifndef TESTDISPLAYHEAPWINDOW_H
#define TESTDISPLAYHEAPWINDOW_H

#include <QObject>

class TestDisplayHeapWindow : public QObject
{
  Q_OBJECT
public:
  //explicit TestDisplayHeapWindow(QObject *parent = 0);

signals:

public slots:

private slots:
  void TestLongDoubleTo96Bits();
  void Test96BitFlipBits();
  void Test96BitSubtraction();
  void Test96BitAddition();
  void Test96ToAndFromConversion();
  void MapFromHeapToScreenMaximumPositiveSizes();
  void MapFromHeapToScreenBottomLeftWindow();
  void MapFromHeapToScreenTopRightWindow();
  //void MapFromHeapToScreenTopLeftWindow();
  //void MapFromHeapToScreenBottomRightWindow();

  //void TestPanning();
};

#endif // TESTDISPLAYHEAPWINDOW_H

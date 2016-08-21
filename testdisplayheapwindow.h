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
  void MapFromHeapToScreenMaximumPositiveSizes();
  void MapFromHeapToScreenNegativeWindow();
};

#endif // TESTDISPLAYHEAPWINDOW_H

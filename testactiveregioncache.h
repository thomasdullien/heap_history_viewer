#ifndef TESTACTIVEREGIONCACHE_H
#define TESTACTIVEREGIONCACHE_H

#include <QObject>

class TestActiveRegionCache : public QObject
{
  Q_OBJECT
public:

signals:

public slots:

private slots:
  void TestSizeCalculation();
  void TestCacheCoalescing();
};

#endif // TESTACTIVEREGIONCACHE_H

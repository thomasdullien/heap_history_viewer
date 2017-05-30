#include <QtTest/QtTest>

#include "activeregioncache.h"
#include "glsl_simulation_functions.h"
#include "heapwindow.h"
#include "heapblock.h"
#include "testactiveregioncache.h"

void TestActiveRegionCache::TestSizeCalculation() {
  HeapBlock h1(0, 200, 2000, 0xDEADBEEF);
  HeapBlock h2(20, 250, 4000, 0xDEADBEEF+2000 );

  std::vector<HeapBlock> vec = {h1, h2};
  ActiveRegionCache(6000, &vec);
  QCOMPARE(1.0, -1.0);
}

void TestActiveRegionCache::TestCacheCoalescing() {
}



//QTEST_MAIN(TestActiveRegionCache)


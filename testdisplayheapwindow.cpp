#include <QtTest/QtTest>

#include "displayheapwindow.h"
#include "heapwindow.h"
#include "testdisplayheapwindow.h"

/*TestDisplayHeapWindow::TestDisplayHeapWindow(QObject *parent) :
QObject(parent)
{

}*/

//void TestDisplayHeapWindow::Test64BitSubtraction() {
//  ivec2 min_ivec(0, 0);
//  ivec2 subtrahend(0xF >> 1, 0x)
//}

void TestDisplayHeapWindow::MapFromHeapToScreenMaximumPositiveSizes() {
  DisplayHeapWindow display_heap_window;

  ivec3 minimum_address(0, 0, 0);
  // Entire 64-bit address space, left-shifted by 4 bits.
  ivec3 maximum_address(0xFFFFFFF0, 0xFFFFFFFF, 0xF);
  ivec2 minimum_tick(0, 0);
  // 32-bit tick space, left-shifted by 4 bits.
  ivec2 maximum_tick(0xFFFFFFF0, 0xF);

  display_heap_window.setMaximumAddress(maximum_address);
  display_heap_window.setMinimumAddress(minimum_address);
  display_heap_window.setMaximumTick(maximum_tick);
  display_heap_window.setMinimumTick(minimum_tick);

  auto result = display_heap_window.mapHeapCoordinateToDisplay(
      0xFFFFFFFF, 0xFFFFFFFFFFFFFFFFUL);
  QCOMPARE(result.first, 1.0);
  QCOMPARE(result.second, 1.0);

  auto result2 = display_heap_window.mapHeapCoordinateToDisplay(0, 0);
  QCOMPARE(result2.first, -1.0);
  QCOMPARE(result2.second, -1.0);

  auto result3 = display_heap_window.mapHeapCoordinateToDisplay(
      0xFFFFFFFF >> 1, 0xFFFFFFFFFFFFFFFFUL >> 1);
  QCOMPARE(result3.first, 0.0);
  QCOMPARE(result3.second, 0.0);
}

// Sets up a DisplayHeapWindow of maximum size so that only the upper-right
// quarter of the screen falls into the screen space. This involves both
// negative tick and address coordinates.
void TestDisplayHeapWindow::MapFromHeapToScreenNegativeWindow() {
  DisplayHeapWindow display_heap_window;

  ivec3 minimum_address(0, 0, 0);
  // Entire 64-bit address space, left-shifted by 4 bits.
  ivec3 maximum_address(0xFFFFFFF0, 0xFFFFFFFF, 0xF);
  ivec2 minimum_tick(0, 0);
  // 32-bit tick space, left-shifted by 4 bits.
  ivec2 maximum_tick(0xFFFFFFF0, 0xF);

  // Now shift the coordinates left and down.
  ivec3 half_window_height(0xFFFFFFFF8, 0xFFFFFFFF, 0x7);
  ivec2 half_window_width(0xFFFFFFF8, 0x7);

  minimum_address = Sub96(minimum_address, half_window_height);
  maximum_address = Sub96(maximum_address, half_window_height);
  minimum_tick = Sub64(minimum_tick, half_window_width);
  maximum_tick = Sub64(maximum_tick, half_window_width);

  display_heap_window.setMaximumAddress(maximum_address);
  display_heap_window.setMinimumAddress(minimum_address);
  display_heap_window.setMaximumTick(maximum_tick);
  display_heap_window.setMinimumTick(minimum_tick);

  // The origin of the heap coordinate system should now map to the center
  // of the screen.
  auto result2 = display_heap_window.mapHeapCoordinateToDisplay(0, 0);
  // Should now map to the center of the screen.
  QCOMPARE(result2.first, 0.0);
  QCOMPARE(result2.second, 0.0);

  // The middle of the current window should now map to the upper right
  // corner of the screen.
  auto result3 = display_heap_window.mapHeapCoordinateToDisplay(
      0xFFFFFFFF >> 1, 0xFFFFFFFFFFFFFFFFUL >> 1);
  // Shoud now map to the top-right corner.
  QCOMPARE(result3.first, 1.0);
  QCOMPARE(result3.second, 1.0);
}

QTEST_MAIN(TestDisplayHeapWindow)
//#include "testdisplayheapwindow.moc"

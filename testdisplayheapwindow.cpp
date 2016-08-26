#include <QtTest/QtTest>

#include "displayheapwindow.h"
#include "heapwindow.h"
#include "testdisplayheapwindow.h"

void TestDisplayHeapWindow::TestLongDoubleTo96Bits() {
  long double test(2);
  ivec3 result = LongDoubleTo96Bits(test);
  QCOMPARE(result.x , 2);
  QCOMPARE(result.y, 0);
  QCOMPARE(result.z, 0);
  test = 0x100000000;
  result = LongDoubleTo96Bits(test);
  QCOMPARE(result.x, 0);
  QCOMPARE(result.y, 1);
  QCOMPARE(result.z, 0);
}

void TestDisplayHeapWindow::Test96BitFlipBits() {
  ivec3 result;
  result.flipBit(0);
  QCOMPARE(result.x, 1);
  result.flipBit(31);
  QCOMPARE(result.x, static_cast<int32_t>(0x80000001));
  result.flipBit(32);
  QCOMPARE(result.y, 1);
  result.flipBit(63);
  QCOMPARE(result.y, static_cast<int32_t>(0x80000001));
  result.flipBit(64);
  QCOMPARE(result.z, 1);
}

void TestDisplayHeapWindow::Test96BitSubtraction() {
  ivec3 base(0, 0, 0);
  ivec3 subtrahend(0xFFFFFFF8, 0xFFFFFFFF, 0x7);
  ivec3 result = Sub96(base, subtrahend);

  QCOMPARE(static_cast<uint32_t>(result.z), 0xFFFFFFF8);
  QCOMPARE(static_cast<uint32_t>(result.y), 0x00000000U);
  QCOMPARE(static_cast<uint32_t>(result.x), 0x00000008U);

  ivec3 subtrahend2(0x43232151, 0x7FFFF, 0);
  ivec3 value(0x631C4000, 0x7FFFF, 0);
  result = Sub96(value, subtrahend2);

  QCOMPARE(static_cast<uint32_t>(result.x), 0x1FF91EAFU);
  QCOMPARE(static_cast<uint32_t>(result.y), 0U);
}

void TestDisplayHeapWindow::Test96BitAddition() {
  ivec3 base(0, 0, 0);
  ivec3 addend(0xFFFFFFF8, 0xFFFFFFFF, 0x7);
  ivec3 result = Add96(base, addend);

  QCOMPARE(static_cast<uint32_t>(result.z), 0x7U);
  QCOMPARE(static_cast<uint32_t>(result.y), 0xFFFFFFFFU);
  QCOMPARE(static_cast<uint32_t>(result.x), 0xFFFFFFF8U);

  ivec3 base2(0xFFFFFFF0, 0xFFFFFFFF, 0xF);
  ivec3 result2 = Add96(base2, addend);

  QCOMPARE(static_cast<uint32_t>(result2.z), 0x17U);
  QCOMPARE(static_cast<uint32_t>(result2.y), 0xFFFFFFFFU);
  QCOMPARE(static_cast<uint32_t>(result2.x), 0xFFFFFFE8U);
}

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
void TestDisplayHeapWindow::MapFromHeapToScreenBottomLeftWindow() {
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

// Sets up a DisplayHeapWindow of maximum size so that only the lower-left
// quarter of the screen falls into the screen space. This involves very
// large tick and address coordinates.
void TestDisplayHeapWindow::MapFromHeapToScreenTopRightWindow() {
  DisplayHeapWindow display_heap_window;

  ivec3 minimum_address(0, 0, 0);
  // Entire 64-bit address space, left-shifted by 4 bits.
  ivec3 maximum_address(0xFFFFFFF0, 0xFFFFFFFF, 0xF);
  ivec2 minimum_tick(0, 0);
  // 32-bit tick space, left-shifted by 4 bits.
  ivec2 maximum_tick(0xFFFFFFF0, 0xF);

  // Now shift the coordinates right and up.
  ivec3 half_window_height(0xFFFFFFFF0, 0xFFFFFFFF, 0x7);
  ivec2 half_window_width(0xFFFFFFF0, 0x7);

  minimum_address = Add96(minimum_address, half_window_height);
  maximum_address = Add96(maximum_address, half_window_height);
  minimum_tick = Add64(minimum_tick, half_window_width);
  maximum_tick = Add64(maximum_tick, half_window_width);

  display_heap_window.setMaximumAddress(maximum_address);
  display_heap_window.setMinimumAddress(minimum_address);
  display_heap_window.setMaximumTick(maximum_tick);
  display_heap_window.setMinimumTick(minimum_tick);

  // The origin of the heap coordinate system should now map to the center
  // of the screen.
  auto result2 = display_heap_window.mapHeapCoordinateToDisplay(
      0xFFFFFFFF, 0xFFFFFFFFFFFFFFFF);
  // Should now map to the center of the screen.
  QCOMPARE(result2.first, 0.0);
  QCOMPARE(result2.second, 0.0);

  // The middle of the current window should now map to the lower left
  // corner of the screen.
  auto result3 = display_heap_window.mapHeapCoordinateToDisplay(
      0xFFFFFFFF >> 1, 0xFFFFFFFFFFFFFFFFUL >> 1);
  // Shoud now map to the top-right corner.
  QCOMPARE(result3.first, -1.0);
  QCOMPARE(result3.second, -1.0);
}

QTEST_MAIN(TestDisplayHeapWindow)
//#include "testdisplayheapwindow.moc"

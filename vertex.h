#ifndef VERTEX_H
#define VERTEX_H

#include <cstdint>
#include <cstddef>

#include <QVector3D>

// A vertex for the purposes of the heap visualizer. The requirements
// are a bit different than for a "normal" vertex -- because the heap
// can be 2^64 values high, just using normal floats for y will not
// work. Unfortunately, the GSLS standard 1.3 does not support doubles,
// so using a double is not an option, either.
class HeapVertex {
public:
  HeapVertex(uint32_t x, uint64_t y, const QVector3D &color);

  static inline int positionOffset() { return offsetof(HeapVertex, x_); }
  static inline int colorOffset() { return offsetof(HeapVertex, color_); }
  static inline int stride() { return sizeof(HeapVertex); }

  uint32_t getX() const { return x_; }
  uint64_t getY() const { return (static_cast<uint64_t>(y2_) << 32) + y1_; }
  const QVector3D &getColor() const { return color_; }

  static const int PositionTupleSize = 3;
  static const int ColorTupleSize = 3;

private:
  uint32_t x_;
  uint32_t y1_;
  uint32_t y2_;
  QVector3D color_;
};

#endif // VERTEX_H

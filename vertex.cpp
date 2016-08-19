#include "vertex.h"

HeapVertex::HeapVertex(uint32_t x, uint64_t y, const QVector3D &color) :
    x_(x), y1_(y), y2_(y >> 32), color_(color) {};

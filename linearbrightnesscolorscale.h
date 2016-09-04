#ifndef LINEARBRIGHTNESSCOLORSCALE_H
#define LINEARBRIGHTNESSCOLORSCALE_H

#include <QVector3D>

class LinearBrightnessColorScale {
public:
  static std::pair<QVector3D, QVector3D> allocatedColorsFromTick(uint32_t allocation_tick, uint32_t maximum_tick);
  static std::pair<QVector3D, QVector3D> freedColorsFromTick(uint32_t allocation_tick, uint32_t maximum_tick);
  static std::pair<QVector3D, QVector3D> colorsFromTick(uint32_t allocation_tick, uint32_t end_tick, uint32_t maximum_tick);
private:
  static float colorHueScaled(uint32_t tick, uint32_t max_tick, float range_low, float range_high);
};

#endif // LINEARBRIGHTNESSCOLORSCALE_H

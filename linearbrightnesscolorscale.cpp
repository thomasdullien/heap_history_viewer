#include "linearbrightnesscolorscale.h"

// Maps from [0, max_tick] -> [range_low, range_high] linearly..
float LinearBrightnessColorScale::colorHueScaled(uint32_t tick, uint32_t max_tick, float range_low, float range_high) {
  float range_length = range_high-range_low;
  double scaled = static_cast<double>(tick) / static_cast<double>(max_tick);
  return range_low + (scaled * range_length);
}

// Provides a color scale of yellow-orange hues for still-allocated blocks.
std::pair<QVector3D, QVector3D> LinearBrightnessColorScale::allocatedHighlightColorsFromTick(uint32_t allocation_tick, uint32_t maximum_tick) {
    float scaled_value = colorHueScaled(allocation_tick, maximum_tick, 0.4f, 0.9f);
    const float value_lighter = scaled_value + 0.1;
    return std::make_pair(QVector3D(scaled_value, scaled_value, 0.0), QVector3D(value_lighter, value_lighter, 0.0));
}

// Provides a color scale of reddish-orange hues for freed blocks.
std::pair<QVector3D, QVector3D> LinearBrightnessColorScale::freedHighlightColorsFromTick(uint32_t allocation_tick, uint32_t maximum_tick) {
    float scaled_value = colorHueScaled(allocation_tick, maximum_tick, 0.0f, 0.6f);
    const float value_lighter = scaled_value + 0.1;
    return std::make_pair(QVector3D(scaled_value + 0.3, scaled_value, 0.0), QVector3D(value_lighter + 0.3, value_lighter, 0.0));
}

// Provides a color scale of green hues for still-allocated blocks.
std::pair<QVector3D, QVector3D> LinearBrightnessColorScale::allocatedColorsFromTick(uint32_t allocation_tick, uint32_t maximum_tick) {
  float scaled_value = colorHueScaled(allocation_tick, maximum_tick, 0.4f, 0.9f);
  const float value_lighter = scaled_value + 0.1;
  return std::make_pair(QVector3D(0.0, scaled_value, 0.0), QVector3D(0.0, value_lighter, 0.0));
}

// Provides a color scale of grey hues for freed blocks.
std::pair<QVector3D, QVector3D> LinearBrightnessColorScale::freedColorsFromTick(uint32_t allocation_tick, uint32_t maximum_tick) {
  float scaled_value = colorHueScaled(allocation_tick, maximum_tick, 0.0f, 0.7f);
  const float value_lighter = scaled_value + 0.2;
  return std::make_pair(QVector3D(scaled_value, scaled_value, scaled_value), QVector3D(value_lighter, value_lighter, value_lighter));
}

std::pair<QVector3D, QVector3D> LinearBrightnessColorScale::colorsFromTick(uint32_t allocation_tick, uint32_t end_tick, uint32_t maximum_tick) {
  if (end_tick == std::numeric_limits<uint32_t>::max()) {
    return allocatedColorsFromTick(allocation_tick, maximum_tick);
  } else {
    return freedColorsFromTick(allocation_tick, maximum_tick);
  }
}

std::pair<QVector3D, QVector3D> LinearBrightnessColorScale::highlightedColorsFromTick(uint32_t allocation_tick, uint32_t end_tick, uint32_t maximum_tick) {
  if (end_tick == std::numeric_limits<uint32_t>::max()) {
    return allocatedHighlightColorsFromTick(allocation_tick, maximum_tick);
  } else {
    return freedHighlightColorsFromTick(allocation_tick, maximum_tick);
  }
}

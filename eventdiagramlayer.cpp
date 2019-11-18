#include "eventdiagramlayer.h"

EventDiagramLayer::EventDiagramLayer() :
  GLHeapDiagramLayer(":/event_shader.vert", ":/simple.frag", true) {
}

void EventDiagramLayer::loadVerticesFromHeapHistory(const HeapHistory& history, bool) {
  std::vector<HeapVertex> *vertices = getVertexVector();
  vertices->clear();
  history.eventsToVertices(vertices);
}

std::pair<vec4, vec4> EventDiagramLayer::vertexShaderSimulator(const HeapVertex& vertex) {
  ivec3 position(vertex.getX(), vertex.getY() & 0xFFFFFFFF, vertex.getY() >> 32u);
  int visible_heap_base_A = visible_heap_base_A_;
  int visible_heap_base_B = visible_heap_base_B_;
  int visible_heap_base_C = visible_heap_base_C_;
  int visible_tick_base_A = visible_tick_base_A_;
  int visible_tick_base_B = visible_tick_base_B_;
  float scale_heap_x = vertex_to_screen_.data()[0];
  float scale_heap_y = vertex_to_screen_.data()[2];
  float scale_heap_to_screen[2][2] = {{scale_heap_x, 0.0}, {0.0, scale_heap_y}};
  vec3 color(vertex.getColor().x(), vertex.getColor().y(), vertex.getColor().z());

  Q_UNUSED(visible_heap_base_A);
  Q_UNUSED(visible_heap_base_B);
  Q_UNUSED(visible_heap_base_C);

  // =========================================================================
  // Everything below should be valid C++ and also valid GLSL! This code is
  // shared between displayheapwindow.cpp and simple.vert, so make sure it
  // always stays in synch!!
  // =========================================================================
  //
  // Read the X (tick) and Y (address) coordinate of the current point.
  ivec2 tick = Load32BitLeftShiftedBy4Into64Bit(position.x);
  // Lowest 4 bit represent fractional component, again.
  ivec2 minimum_visible_tick = ivec2(visible_tick_base_A, visible_tick_base_B);
  // Translate the x / tick coordinate to be aligned with 0.
  ivec2 tick_coordinate_translated = Sub64(tick, minimum_visible_tick);

  float temp_x = Multiply64BitWithFloat(tick_coordinate_translated,
                                          scale_heap_to_screen[0][0]);

  float final_x = temp_x * scale_heap_to_screen[0][0];
  float final_y = -1.0;
  if (position.y != 0) {
    final_y = 1.0;
  }
  final_x = 2 * final_x - 1;

  // ==========================================================================
  // End of mandatory valid GLSL part.
  // ==========================================================================

  vec4 gl_Position = vec4(final_x, final_y, 0.0, 1.0);
  vec4 vColor = vec4(color, 0.5);
  return std::make_pair(gl_Position, vColor);
}


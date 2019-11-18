#include "heapblockdiagramlayer.h"

HeapBlockDiagramLayer::HeapBlockDiagramLayer() :
  GLHeapDiagramLayer(":/simple.vert", ":/simple.frag", false) {
}

void HeapBlockDiagramLayer::loadVerticesFromHeapHistory(const HeapHistory& history, bool all) {
  std::vector<HeapVertex> *vertices = getVertexVector();
  vertices->clear();

  history.heapBlockVerticesForActiveWindow(vertices, all);
}

std::pair<vec4, vec4> HeapBlockDiagramLayer::vertexShaderSimulator(const HeapVertex& vertex) {
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

  // =========================================================================
  // Everything below should be valid C++ and also valid GLSL! This code is
  // shared between heapblockdiagramlayer.cpp and simple.vert, so make sure it
  // always stays in synch!!
  // =========================================================================

  // Read the X (tick) and Y (address) coordinate of the current point.
  ivec2 tick = Load32BitLeftShiftedBy4Into64Bit(position.x);
  ivec3 address = Load64BitLeftShiftedBy4Into96Bit(position.y, position.z);

  // Get the base of the heap in the displayed window. This is a 96-bit number
  // where the lowest 4 bit represent a fractional component, the rest is a
  // normal 92-bit integer.
  ivec3 heap_base =
      ivec3(visible_heap_base_A, visible_heap_base_B, visible_heap_base_C);

  // Translate the y / address coordinate of the heap so that the left lower
  // corner of the visible heap window aligns with 0.
  ivec3 address_coordinate_translated = Sub96(address, heap_base);

  // Lowest 4 bit represent fractional component, again.
  ivec2 minimum_visible_tick = ivec2(visible_tick_base_A, visible_tick_base_B);

  // Translate the x / tick coordinate to be aligned with 0.
  ivec2 tick_coordinate_translated = Sub64(tick, minimum_visible_tick);

  // Multiply the y coordinate with the y entry of the transformation matrix.
  // To avoid a degenerate matrix, C++ code supplies a matrix containing the
  // square roots of the actual matrix to the shader code, so apply the float
  // twice
  float temp_y = Multiply96BitWithFloat(address_coordinate_translated,
                                        scale_heap_to_screen[1][1]);
  float final_y = temp_y * scale_heap_to_screen[1][1];

  float temp_x = Multiply64BitWithFloat(tick_coordinate_translated,
                                        scale_heap_to_screen[0][0]);
  float final_x = temp_x * scale_heap_to_screen[0][0];

  final_y = 2 * final_y - 1;
  final_x = 2 * final_x - 1;
  // ==========================================================================
  // End of mandatory valid GLSL part.
  // ==========================================================================

  vec4 gl_Position = vec4(final_x, final_y, 0.0, 1.0);
  // For debugging, uncomment the following line.
  //gl_Position = vec4(color.r, color.g, 0.0, 1.0);
  //  vColor = IntToColor(FloatToInt(scale_heap_to_screen[1][1] * 255 * 255 * 255));
  //if (scale_heap_to_screen[1][1] > 0.1051) {
  //    vColor = vec4(1.0, 0.0, 0.0, 1.0);
  //} else {
  //    vColor = vec4(0.0, 1.0, 0.0, 1.0);
  //}
  vec4 vColor = vec4(color, 1.0);

  return std::make_pair(gl_Position, vColor);
}

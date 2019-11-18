#include "addressdiagramlayer.h"
#include "heaphistory.h"

AddressDiagramLayer::AddressDiagramLayer() :
  GLHeapDiagramLayer(":/address_shader.vert", ":/simple.frag", true) {
}

void AddressDiagramLayer::loadVerticesFromHeapHistory(const HeapHistory& history, bool) {
  std::vector<HeapVertex> *vertices = getVertexVector();
  vertices->clear();
  history.addressesToVertices(vertices);
}

std::pair<vec4, vec4> AddressDiagramLayer::vertexShaderSimulator(const HeapVertex& vertex) {
  ivec3 position(vertex.getX(), vertex.getY() & 0xFFFFFFFF, vertex.getY() >> 32);
  int visible_heap_base_A = visible_heap_base_A_;
  int visible_heap_base_B = visible_heap_base_B_;
  int visible_heap_base_C = visible_heap_base_C_;
  int visible_tick_base_A = visible_tick_base_A_;
  int visible_tick_base_B = visible_tick_base_B_;
  float *matrix_data = vertex_to_screen_.data();
  float scale_heap_x = matrix_data[0];
  float scale_heap_y = matrix_data[3];
  float scale_heap_to_screen[2][2] = {{scale_heap_x, 0.0}, {0.0, scale_heap_y}};
  vec3 color(vertex.getColor().x(), vertex.getColor().y(), vertex.getColor().z());

  Q_UNUSED(visible_tick_base_A);
  Q_UNUSED(visible_tick_base_B);

  // =========================================================================
  // Everything below should be valid C++ and also valid GLSL! This code is
  // shared between addressdiagramlayer.cpp and address_shader.vert, so make
  // sure it always stays in synch!!
  // =========================================================================
  //
  // Read the X (tick) and Y (address) coordinate of the current point.
  ivec3 address = Load64BitLeftShiftedBy4Into96Bit(position.y, position.z);

  // Get the base of the heap in the displayed window. This is a 96-bit number
  // where the lowest 4 bit represent a fractional component, the rest is a
  // normal 92-bit integer.
  ivec3 heap_base =
      ivec3(visible_heap_base_A, visible_heap_base_B, visible_heap_base_C);

  // Translate the y / address coordinate of the heap so that the left lower
  // corner of the visible heap window aligns with 0.
  ivec3 address_coordinate_translated = Sub96(address, heap_base);
printf(" (address_coordinate_translated %s) ", ivec3ToHex(address_coordinate_translated).c_str());
  // Multiply the y coordinate with the y entry of the transformation matrix.
  // To avoid a degenerate matrix, C++ code supplies a matrix containing the
  // square roots of the actual matrix to the shader code, so apply the float
  // twice
  float temp_y = Multiply96BitWithFloat(address_coordinate_translated,
                                        scale_heap_to_screen[1][1]);
  float final_y = temp_y * scale_heap_to_screen[1][1];

  final_y = 2 * final_y - 1;

  float final_x = -1.0;
  if (position.x != 0) {
    final_x = 1.0;
  }
  // ==========================================================================
  // End of mandatory valid GLSL part.
  // ==========================================================================

  vec4 gl_Position = vec4(final_x, final_y, 0.0, 1.0);
  vec4 vColor = vec4(color, 0.5);

  return std::make_pair(gl_Position, vColor);
}

// A "layer" in the heap diagram - the actual diagram for the user is composed
// of several such layers (heap blocks, horizontal lines ~ 'addresses', vertical
// lines ~ 'events). Since each layer has to perform extremely similar actions
// but call different shaders with different geometry, most of the base case is
// handled in this class.

#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>

#include "glheapdiagramlayer.h"

// Users of this class need to provide a vertex and fragment shader program, and
// indicate if the layer is a lines-only layer or of actual triangles / rectangles
// ought to be drawn.
GLHeapDiagramLayer::GLHeapDiagramLayer(
    const std::string &vertex_shader_name,
    const std::string &fragment_shader_name, bool is_line_layer)
    : vertex_shader_name_(vertex_shader_name),
      fragment_shader_name_(fragment_shader_name),
      layer_shader_program_(new QOpenGLShaderProgram()),
      is_line_layer_(is_line_layer) {}

GLHeapDiagramLayer::~GLHeapDiagramLayer() {
  if (is_initialized_) {
    layer_vao_.destroy();
    layer_vertex_buffer_.destroy();
  }
}

// Mostly boilerplate code for OpenGL -- load the shaders, link and bind them,
// create a vertex etc.
void GLHeapDiagramLayer::initializeGLStructures(QOpenGLFunctions *parent) {
  // Load the shaders.
  layer_shader_program_->addShaderFromSourceFile(QOpenGLShader::Vertex,
                                                 vertex_shader_name_.c_str());
  layer_shader_program_->addShaderFromSourceFile(QOpenGLShader::Fragment,
                                                 fragment_shader_name_.c_str());
  layer_shader_program_->link();
  layer_shader_program_->bind();

  // Create the heap block vertex buffer.
  layer_vertex_buffer_.create();
  layer_vertex_buffer_.bind();
  layer_vertex_buffer_.setUsagePattern(QOpenGLBuffer::StaticDraw);

  // The vertex buffer ought to be filled now..
  layer_vertex_buffer_.allocate(&(layer_vertices_[0]),
                                layer_vertices_.size() * sizeof(HeapVertex));

  setupStandardUniforms();

  // Create the vertex array object.
  layer_vao_.create();
  layer_vao_.bind();

  // Register attribute arrays with stride for the vertex attributes.
  layer_shader_program_->enableAttributeArray(0);
  layer_shader_program_->enableAttributeArray(1);
  layer_shader_program_->setAttributeBuffer(
      0, GL_FLOAT, HeapVertex::positionOffset(), HeapVertex::PositionTupleSize,
      HeapVertex::stride());
  layer_shader_program_->setAttributeBuffer(
      1, GL_FLOAT, HeapVertex::colorOffset(), HeapVertex::ColorTupleSize,
      HeapVertex::stride());
  parent->glBindAttribLocation(layer_shader_program_->programId(), 0,
                               "position");
  parent->glBindAttribLocation(layer_shader_program_->programId(), 1, "color");

  // Unbind.
  layer_vao_.release();
  layer_vertex_buffer_.release();
  layer_shader_program_->release();

  is_initialized_ = true;
}

void GLHeapDiagramLayer::setupStandardUniforms() {
  uniform_vertex_to_screen_ =
      layer_shader_program_->uniformLocation("scale_heap_to_screen");
  // The reason why three numbers are needed for the visible heap base
  // is that the central heap rectangle can in theory range from
  // 0 to 2^64-1. This means that the minimum address that can be on
  // screen can be as low as -2^63 (e.g. half a maximum-sized heap down)
  // or as high as 2^64+2^63.
  uniform_visible_heap_base_A_ =
      layer_shader_program_->uniformLocation("visible_heap_base_A");
  uniform_visible_heap_base_B_ =
      layer_shader_program_->uniformLocation("visible_heap_base_B");
  uniform_visible_heap_base_C_ =
      layer_shader_program_->uniformLocation("visible_heap_base_C");
  // The lowest visible tick is passed as 64 bits (e.g. two 32-bit numbers).
  uniform_visible_tick_base_A_ =
      layer_shader_program_->uniformLocation("visible_tick_base_A");
  uniform_visible_tick_base_B_ =
      layer_shader_program_->uniformLocation("visible_tick_base_B");
}

// Set the heap base.
void GLHeapDiagramLayer::setHeapBaseUniforms(int32_t x, int32_t y, int32_t z) {
  layer_shader_program_->setUniformValue(uniform_visible_heap_base_A_, x);
  layer_shader_program_->setUniformValue(uniform_visible_heap_base_B_, y);
  layer_shader_program_->setUniformValue(uniform_visible_heap_base_C_, z);
}

// Sends the value of the matrix that performs heap-space-to-screen-space
// transformation to the shader.
void GLHeapDiagramLayer::setHeapToScreenMatrix(
    const QMatrix2x2 &heap_to_screen) {
  layer_shader_program_->setUniformValue(uniform_vertex_to_screen_,
                                         heap_to_screen);
}

void GLHeapDiagramLayer::paintLayer(ivec2 tick, ivec3 address,
                                    const QMatrix2x2 &heap_to_screen) {
  layer_shader_program_->bind();
  setHeapToScreenMatrix(heap_to_screen);
  setHeapBaseUniforms(address.x, address.y, address.z);
  setTickBaseUniforms(tick.x, tick.y);

  {
    layer_vao_.bind();
    glDrawArrays(is_line_layer_ ? GL_LINES : GL_TRIANGLES, 0,
                 layer_vertices_.size() * sizeof(HeapVertex));
    layer_vao_.release();
  }
  layer_shader_program_->release();
}

void GLHeapDiagramLayer::setTickBaseUniforms(int32_t x, int32_t y) {
  layer_shader_program_->setUniformValue(uniform_visible_tick_base_A_, x);
  layer_shader_program_->setUniformValue(uniform_visible_tick_base_B_, y);
}

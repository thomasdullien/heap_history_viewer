#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>

#include "glheapdiagramlayer.h"

GLHeapDiagramLayer::GLHeapDiagramLayer(const std::string &vertex_shader_name,
                                       const std::string &fragment_shader_name)
    : vertex_shader_name_(vertex_shader_name),
      fragment_shader_name_(fragment_shader_name),
      layer_shader_program_(new QOpenGLShaderProgram()) {}

GLHeapDiagramLayer::~GLHeapDiagramLayer() {
  if (is_initialized_) {
    layer_vao_.destroy();
    layer_vertex_buffer_.destroy();
  }
}

void GLHeapDiagramLayer::initializeGLStructures(QOpenGLFunctions* parent) {
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
  parent->glBindAttribLocation(layer_shader_program_->programId(), 0, "position");
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
  uniform_visible_heap_base_A_ =
      layer_shader_program_->uniformLocation("visible_heap_base_A");
  uniform_visible_heap_base_B_ =
      layer_shader_program_->uniformLocation("visible_heap_base_B");
  uniform_visible_heap_base_C_ =
      layer_shader_program_->uniformLocation("visible_heap_base_C");
  uniform_visible_tick_base_A_ =
      layer_shader_program_->uniformLocation("visible_tick_base_A");
  uniform_visible_tick_base_B_ =
      layer_shader_program_->uniformLocation("visible_tick_base_B");
}

void GLHeapDiagramLayer::setHeapBaseUniforms(int32_t x, int32_t y, int32_t z) {
  layer_shader_program_->setUniformValue(uniform_visible_heap_base_A_, x);
  layer_shader_program_->setUniformValue(uniform_visible_heap_base_B_, y);
  layer_shader_program_->setUniformValue(uniform_visible_heap_base_C_, z);
}

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
    glDrawArrays(GL_TRIANGLES, 0, layer_vertices_.size() * sizeof(HeapVertex));
    layer_vao_.release();
  }
  layer_shader_program_->release();
}

void GLHeapDiagramLayer::setTickBaseUniforms(int32_t x, int32_t y) {
  layer_shader_program_->setUniformValue(uniform_visible_tick_base_A_, x);
  layer_shader_program_->setUniformValue(uniform_visible_tick_base_B_, y);
}

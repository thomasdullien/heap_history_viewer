#include <math.h>

#include <iostream>
#include <fstream>

#include <QApplication>
#include <QtGlobal>
#include <QOpenGLShaderProgram>
#include <QMouseEvent>
#include <QWindow>

#include "heapblock.h"
#include "vertex.h"
#include "glheapdiagram.h"

GLHeapDiagram::GLHeapDiagram(QWidget *parent)
    : QOpenGLWidget(parent), heap_shader_program_(new QOpenGLShaderProgram()),
      grid_shader_(new QOpenGLShaderProgram()) {

  //  QObject::connect(this, SIGNAL(blockClicked), parent->parent(),
  //  SLOT(blockClicked));
}

// Vertices for drawing the background grid.
static std::vector<HeapVertex> g_grid_vertices;

// Vertices for drawing the heap layout.
static std::vector<HeapVertex> g_vertices;

void GLHeapDiagram::setupGridGLStructures() {
  // Build a 16x16 unit grid.
  static constexpr QVector3D GREY(0.8f, 0.8f, 0.8f);
  for (uint32_t index = 0; index < number_of_grid_lines_; ++index) {
    // Horizontal lines.
    g_grid_vertices.push_back(
        HeapVertex(0.0f, index * (1.0 / number_of_grid_lines_), GREY));
    g_grid_vertices.push_back(
        HeapVertex(1.0f, index * (1.0 / number_of_grid_lines_), GREY));
    // Vertical lines.
    g_grid_vertices.push_back(
        HeapVertex(index * (1.0 / number_of_grid_lines_), 0.0f, GREY));
    g_grid_vertices.push_back(
        HeapVertex(index * (1.0 / number_of_grid_lines_), 1.0f, GREY));
  }

  grid_shader_->addShaderFromSourceFile(QOpenGLShader::Vertex, ":/grid.vert");
  grid_shader_->addShaderFromSourceFile(QOpenGLShader::Fragment, ":/grid.frag");
  grid_shader_->link();
  grid_shader_->bind();

  // Create the grid vertex buffer and fill it.
  grid_vertex_buffer_.create();
  grid_vertex_buffer_.bind();
  grid_vertex_buffer_.setUsagePattern(QOpenGLBuffer::DynamicDraw);
  grid_vertex_buffer_.allocate(&(g_grid_vertices[0]),
                               g_grid_vertices.size() * sizeof(HeapVertex));

  uniform_grid_to_heap_map_ = grid_shader_->uniformLocation("grid_to_heap_map");
  uniform_heap_to_screen_map_ =
      grid_shader_->uniformLocation("heap_to_screen_map");
  uniform_grid_to_heap_translation_ =
      grid_shader_->uniformLocation("grid_to_heap_translation");

  grid_vao_.create();
  grid_vao_.bind();

  // Register attribute arrays with stride for the vertex attributes.
  grid_shader_->enableAttributeArray(0);
  grid_shader_->enableAttributeArray(1);
  grid_shader_->setAttributeBuffer(0, GL_FLOAT, HeapVertex::positionOffset(),
                                   HeapVertex::PositionTupleSize,
                                   HeapVertex::stride());
  grid_shader_->setAttributeBuffer(1, GL_FLOAT, HeapVertex::colorOffset(),
                                   HeapVertex::ColorTupleSize,
                                   HeapVertex::stride());
  glBindAttribLocation(grid_shader_->programId(), 0, "position");
  glBindAttribLocation(grid_shader_->programId(), 1, "color");

  // Unbind.
  grid_vao_.release();
  grid_vertex_buffer_.release();
  grid_shader_->release();
}

void GLHeapDiagram::setupUniformsForShaders() {
  uniform_vertex_to_screen_ =
      heap_shader_program_->uniformLocation("scale_heap_to_screen");
  uniform_visible_heap_base_A_ =
      heap_shader_program_->uniformLocation("visible_heap_base_A");
  uniform_visible_heap_base_B_ =
      heap_shader_program_->uniformLocation("visible_heap_base_B");
  uniform_visible_heap_base_C_ =
      heap_shader_program_->uniformLocation("visible_heap_base_C");
  uniform_visible_tick_base_A_ =
      heap_shader_program_->uniformLocation("visible_tick_base_A");
  uniform_visible_tick_base_B_ =
      heap_shader_program_->uniformLocation("visible_tick_base_B");
}

void GLHeapDiagram::setupHeapblockGLStructures() {
  // Load the shaders.
  heap_shader_program_->addShaderFromSourceFile(QOpenGLShader::Vertex,
                                                ":/simple.vert");
  heap_shader_program_->addShaderFromSourceFile(QOpenGLShader::Fragment,
                                                ":/simple.frag");
  heap_shader_program_->link();
  heap_shader_program_->bind();

  // Create the heap block vertex buffer.
  heap_vertex_buffer_.create();
  heap_vertex_buffer_.bind();
  heap_vertex_buffer_.setUsagePattern(QOpenGLBuffer::StaticDraw);

  // Fill the vertex buffer from the heap history.
  heap_history_.dumpVerticesForActiveWindow(&g_vertices);
  heap_vertex_buffer_.allocate(&(g_vertices[0]),
                               g_vertices.size() * sizeof(HeapVertex));

  for (const HeapVertex &vertex : g_vertices) {
    printf("[!] Vertex is %lx, %lx\n", vertex.getX(), vertex.getY());
  }
  fflush(stdout);

  setupUniformsForShaders();

  // Create the vertex array object.
  heap_block_vao_.create();
  heap_block_vao_.bind();

  // Register attribute arrays with stride for the vertex attributes.
  heap_shader_program_->enableAttributeArray(0);
  heap_shader_program_->enableAttributeArray(1);
  heap_shader_program_->setAttributeBuffer(
      0, GL_FLOAT, HeapVertex::positionOffset(), HeapVertex::PositionTupleSize,
      HeapVertex::stride());
  heap_shader_program_->setAttributeBuffer(
      1, GL_FLOAT, HeapVertex::colorOffset(), HeapVertex::ColorTupleSize,
      HeapVertex::stride());
  glBindAttribLocation(heap_shader_program_->programId(), 0, "position");
  glBindAttribLocation(heap_shader_program_->programId(), 1, "color");

  // Unbind.
  heap_block_vao_.release();
  heap_vertex_buffer_.release();
  heap_shader_program_->release();
}

void GLHeapDiagram::initializeGL() {
  initializeOpenGLFunctions();
  glEnable(GL_BLEND);
  //  glEnable(GL_CULL_FACE);
  glClearColor(0.0f, 0.0f, 0.0f, 1.0f);

  // Load the heap history.
  std::ifstream ifs("/tmp/heap.json", std::fstream::in);
  //heap_history_.LoadFromJSONStream(ifs);

  uint64_t base = 0x100000000;
  for (int i = 0; i < 10; ++i) {
  heap_history_.recordMalloc(0x200000 + base, 0x200);
    heap_history_.recordMalloc(0x200204+base, 0x200);
    heap_history_.recordMalloc(0x200408+base, 0x200);
  heap_history_.recordFree(0x200000 + base);
    heap_history_.recordFree(0x200408+base);
    heap_history_.recordFree(0x200204+base);
    heap_history_.recordMalloc(0x200200+base, 0x100);
    heap_history_.recordMalloc(0x200304+base, 0x100);
    if (i != 9) {
      heap_history_.recordFree(0x200200+base);
      heap_history_.recordFree(0x200304+base);
      }
  }
  //heap_history_.recordMalloc(140737323938016 >> 15, 0x200);*/
  heap_history_.setCurrentWindowToGlobal();

  setupHeapblockGLStructures();
  setupGridGLStructures();
}

QSize GLHeapDiagram::minimumSizeHint() { return QSize(500, 500); }

QSize GLHeapDiagram::sizeHint() { return QSize(1024, 1024); }

void GLHeapDiagram::updateHeapToScreenMap() {
  double y_scaling;
  double x_scaling;
  getScaleFromHeapToScreen(&x_scaling, &y_scaling);

  heap_to_screen_matrix_.data()[0] = x_scaling;
  heap_to_screen_matrix_.data()[3] = y_scaling;
}

void GLHeapDiagram::getScaleFromHeapToScreen(double *scale_x, double *scale_y) {
  long double y_scaling =
      heap_history_.getCurrentWindow().getYScalingHeapToScreen();
  long double x_scaling =
      heap_history_.getCurrentWindow().getXScalingHeapToScreen();
  *scale_x = x_scaling;
  *scale_y = y_scaling;
}

bool GLHeapDiagram::screenToHeap(double x, double y, uint32_t *tick,
                                 uint64_t *address) {
  return heap_history_.getCurrentWindow().mapDisplayCoordinateToHeap(x, y, tick,
                                                                     address);
}

void GLHeapDiagram::setHeapBaseUniforms() {
  heap_shader_program_->setUniformValue(
      uniform_visible_heap_base_A_,
      heap_history_.getCurrentWindow().getMinimumAddress().x);
  heap_shader_program_->setUniformValue(
      uniform_visible_heap_base_B_,
      heap_history_.getCurrentWindow().getMinimumAddress().y);
  heap_shader_program_->setUniformValue(
      uniform_visible_heap_base_C_,
      heap_history_.getCurrentWindow().getMinimumAddress().z);
}

void GLHeapDiagram::setTickBaseUniforms() {
  heap_shader_program_->setUniformValue(
      uniform_visible_tick_base_A_,
      heap_history_.getCurrentWindow().getMinimumTick().x);
  heap_shader_program_->setUniformValue(
      uniform_visible_tick_base_B_,
      heap_history_.getCurrentWindow().getMinimumTick().y);
}

void GLHeapDiagram::debugDumpVerticesAndMappings() {
  /*
  for (const HeapVertex &vertex : g_vertices) {
    std::pair<float, float> vertex_mapped =
        heap_history_.getCurrentWindow().mapHeapCoordinateToDisplay(
            vertex.getX(), vertex.getY());

    printf("[Debug] Vertex at %d, %ld -> %f %f\n", vertex.getX(), vertex.getY(),
           vertex_mapped.first, vertex_mapped.second);
  }
  printf("[Debug] ----\n");
  fflush(stdout);*/
}

void GLHeapDiagram::paintGL() {
  glClear(GL_COLOR_BUFFER_BIT);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  updateHeapToScreenMap();

  debugDumpVerticesAndMappings();

  // Render the heap blocks.
  heap_shader_program_->bind();

  heap_shader_program_->setUniformValue(uniform_vertex_to_screen_,
                                        heap_to_screen_matrix_);
  // Set the uniforms for the shader to transmit the base address of the
  // currently visible heap.
  setHeapBaseUniforms();

  // Set the uniforms for the shader to transmit the base tick of the
  // current visible portion of the heap history.
  setTickBaseUniforms();

  {
    heap_block_vao_.bind();
    glDrawArrays(GL_TRIANGLES, 0, g_vertices.size() * sizeof(HeapVertex));
    heap_block_vao_.release();
  }
  heap_shader_program_->release();
  /*
    printf("height is %lx, width is %lx\n",
    heap_history_.getCurrentWindow().height(),
    heap_history_.getCurrentWindow().width());
    printf("minimum_address is %lx, maximum_address is %lx\n",
    heap_history_.getCurrentWindow().getMinimumAddress(),
    heap_history_.getCurrentWindow().getMaximumAddress());
    for (const HeapVertex &vertex : g_vertices) {
      double x, y;
      heapToScreen(vertex.getX(), vertex.getY(), &x, &y);
      printf("[!] Vertex is %lx, %lx -> %f %f\n", vertex.getX(), vertex.getY(),
             x, y);
    }
    fflush(stdout);
  */

  /*
  // Render the grid.
  updateUnitSquareToHeapMap();
  grid_shader_->bind();
  grid_shader_->setUniformValue(uniform_heap_to_screen_map_,
                                heap_to_screen_matrix_);
  grid_shader_->setUniformValue(uniform_grid_to_heap_translation_,
                                unit_square_to_heap_translation_);
  grid_shader_->setUniformValue(uniform_grid_to_heap_map_,
                                unit_square_to_heap_matrix_);
  grid_shader_->setUniformValue(uniform_heap_to_screen_translation_,
                                heap_to_screen_translation_);
  {
    grid_vao_.bind();
    glDrawArrays(GL_LINES, 0, g_grid_vertices.size() * sizeof(HeapVertex));
    grid_vao_.release();
  }
  grid_shader_->release();*/
}

void GLHeapDiagram::update() {
  printf("Update called\n");
  QOpenGLWidget::update();
}

void GLHeapDiagram::resizeGL(int w, int h) { printf("Resize GL was called\n"); }

GLHeapDiagram::~GLHeapDiagram() {
  heap_block_vao_.destroy();
  heap_vertex_buffer_.destroy();
  grid_vao_.destroy();
  grid_vertex_buffer_.destroy();
}

void GLHeapDiagram::mousePressEvent(QMouseEvent *event) {
  double x = static_cast<double>(event->x()) / this->width();
  double y = static_cast<double>(event->y()) / this->height();
  uint32_t tick;
  uint64_t address;
  last_mouse_position_ = event->pos();
  if (!screenToHeap(x, y, &tick, &address)) {
    emit showMessage("Click out of bounds.");
    return;
  }
  HeapBlock current_block;
  uint32_t index;

  printf("clicked at tick %d and address %lx\n", tick, address);
  fflush(stdout);

  if (!heap_history_.getBlockAtSlow(address, tick, &current_block, &index)) {

    emit showMessage("Nothing here.");
  } else {
    emit blockClicked(true, current_block);
  }
}

void GLHeapDiagram::mouseMoveEvent(QMouseEvent *event) {
  double dx = event->x() - last_mouse_position_.x();
  double dy = event->y() - last_mouse_position_.y();

  if (event->buttons() & Qt::LeftButton) {
    heap_history_.panCurrentWindow(dx / this->width(), dy / this->height());

    QOpenGLWidget::update();
  } else if (event->buttons() & Qt::RightButton) {
    QOpenGLWidget::update();
  }

  last_mouse_position_ = event->pos();
}

void GLHeapDiagram::wheelEvent(QWheelEvent *event) {
  float movement_quantity = event->angleDelta().y() / 500.0;
  double how_much_y = 1.0;
  double how_much_x = 1.0;
  Qt::KeyboardModifiers modifiers = QApplication::keyboardModifiers();
  double point_x = static_cast<double>(event->x()) / this->width();
  double point_y = static_cast<double>(event->y()) / this->height();

  if (!(modifiers & Qt::ControlModifier) && (modifiers & Qt::ShiftModifier)) {
    how_much_y = 1.0 - movement_quantity;
    heap_history_.zoomToPoint(point_x, point_y, how_much_x, how_much_y);
  } else if ((modifiers & Qt::ControlModifier) &&
             (modifiers & Qt::ShiftModifier)) {
    how_much_x = 1.0 - movement_quantity;
    heap_history_.zoomToPoint(point_x, point_y, how_much_x, how_much_y);
  } else if (modifiers & Qt::ControlModifier) {
    how_much_y = 1.0 - movement_quantity;
    how_much_x = 1.0 - movement_quantity;
    heap_history_.zoomToPoint(point_x, point_y, how_much_x, how_much_y);
  }

  QOpenGLWidget::update();
}

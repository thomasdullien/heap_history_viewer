#include <math.h>

#include <iostream>
#include <fstream>

#include <QApplication>
#include <QFileDialog>
#include <QtGlobal>
#include <QOpenGLShaderProgram>
#include <QMouseEvent>
#include <QWindow>

#include "heapblock.h"
#include "vertex.h"
#include "glheapdiagram.h"

GLHeapDiagram::GLHeapDiagram(QWidget *parent)
    : QOpenGLWidget(parent), block_layer_(new GLHeapDiagramLayer(
                                 ":/simple.vert", ":/simple.frag", false)),
      event_layer_(
          new GLHeapDiagramLayer(":/event_shader.vert", ":/simple.frag", true)),
      address_layer_(new GLHeapDiagramLayer(":/address_shader.vert",
                                            ":/simple.frag", true)),
      is_GL_initialized_(false), file_to_load_("") {

  //  QObject::connect(this, SIGNAL(blockClicked), parent->parent(),
  //  SLOT(blockClicked));
}

void GLHeapDiagram::loadFileInternal() {
  if (is_GL_initialized_) {
    // Load the heap history.
    if (file_to_load_ != "") {
      std::ifstream ifs(file_to_load_, std::fstream::in);
      heap_history_.LoadFromJSONStream(ifs);
    }
    heap_history_.setCurrentWindowToGlobal();

    // Initialize the heap block drawing layer.
    std::vector<HeapVertex> *block_vertices = block_layer_->getVertexVector();
    heap_history_.heapBlockVerticesForActiveWindow(block_vertices);
    block_layer_->initializeGLStructures(this);

    // Initialize the event lines drawing layer.
    std::vector<HeapVertex> *event_vertices = event_layer_->getVertexVector();
    heap_history_.eventsToVertices(event_vertices);
    event_layer_->initializeGLStructures(this);

    // Initialize the address lines drawing layer.
    std::vector<HeapVertex> *address_vertices =
        address_layer_->getVertexVector();
    heap_history_.addressesToVertices(address_vertices);
    address_layer_->initializeGLStructures(this);

    // Initialize the grid lines drawing layer.
  }
}

void GLHeapDiagram::initializeGL() {
  initializeOpenGLFunctions();
  glEnable(GL_BLEND);
  glEnable(GL_CULL_FACE);
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

  is_GL_initialized_ = true;

  loadFileInternal();
}

QSize GLHeapDiagram::minimumSizeHint() { return QSize(500, 500); }

void GLHeapDiagram::setFileToDisplay(QString filename) {
  file_to_load_ = filename.toStdString();
  loadFileInternal();
}

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

void GLHeapDiagram::debugDumpVerticesAndMappings() {
  // heap_history_.getCurrentWindow().setDebug(true);
  /*
  int index = 0;
  for (const HeapVertex &vertex : g_vertices) {
    std::pair<float, float> vertex_mapped =
        heap_history_.getCurrentWindow().mapHeapCoordinateToDisplay(
            vertex.getX(), vertex.getY());

    if ((vertex_mapped.first >= -1.0) && (vertex_mapped.second <= 1.0) &&
        (vertex_mapped.second >= -1.0) && (vertex_mapped.second <= 1.0)) {
            printf("[Debug][Normal] Vertex %d at %d, %lx -> %f %f\n", index,
  vertex.getX(), vertex.getY(),
                   vertex_mapped.first, vertex_mapped.second);
      } else {
             printf("[Debug][Weird!] Vertex %d at %d, %lx -> %f %f\n", index,
  vertex.getX(), vertex.getY(),
                   vertex_mapped.first, vertex_mapped.second);
      }
    ++index;
  }
  printf("[Debug] ----\n");
  fflush(stdout);*/
}

void GLHeapDiagram::paintGL() {
  glClear(GL_COLOR_BUFFER_BIT);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  updateHeapToScreenMap();
  debugDumpVerticesAndMappings();
  const DisplayHeapWindow &heap_window = heap_history_.getCurrentWindow();
  heap_window.setDebug(true);

  block_layer_->paintLayer(heap_window.getMinimumTick(),
                           heap_window.getMinimumAddress(),
                           heap_to_screen_matrix_);

  glLineWidth(1.5f);
  event_layer_->paintLayer(heap_window.getMinimumTick(),
                           heap_window.getMinimumAddress(),
                           heap_to_screen_matrix_);

  heap_window.debugDumpHeapVerticesToAddressMapper(
    address_layer_->getVertexVector());

  address_layer_->paintLayer(heap_window.getMinimumTick(),
                             heap_window.getMinimumAddress(),
                             heap_to_screen_matrix_);
}

void GLHeapDiagram::update() {
  printf("Update called\n");
  QOpenGLWidget::update();
}

void GLHeapDiagram::resizeGL(int w, int h) { printf("Resize GL was called\n"); }

GLHeapDiagram::~GLHeapDiagram() {}

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
    // No block here. Perhaps an event?
    std::string eventstring;
    if (heap_history_.getEventAtTick(tick, &eventstring)) {
      char buf[1024];
      sprintf(buf, "Event at tick %08.08lx: ", tick);
      emit showMessage(std::string(buf) + eventstring);
    } else {
      char buf[1024];
      sprintf(buf, "Nothing here at tick %08.08lx and address %016.16lx", tick,
              address);
      emit showMessage(std::string(buf));
    }
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
  long double max_height =
      (heap_history_.getMaximumAddress() - heap_history_.getMinimumAddress()) *
      1.5 * 16;
  long double max_width =
      (heap_history_.getMaximumTick() - heap_history_.getMinimumTick()) * 1.5 *
      16;

  if (!(modifiers & Qt::ControlModifier) && (modifiers & Qt::ShiftModifier)) {
    how_much_y = 1.0 - movement_quantity;
    heap_history_.zoomToPoint(point_x, point_y, how_much_x, how_much_y,
                              max_height, max_width);
  } else if ((modifiers & Qt::ControlModifier) &&
             (modifiers & Qt::ShiftModifier)) {
    how_much_x = 1.0 - movement_quantity;
    heap_history_.zoomToPoint(point_x, point_y, how_much_x, how_much_y,
                              max_height, max_width);
  } else if (modifiers & Qt::ControlModifier) {
    how_much_y = 1.0 - movement_quantity;
    how_much_x = 1.0 - movement_quantity;
    heap_history_.zoomToPoint(point_x, point_y, how_much_x, how_much_y,
                              max_height, max_width);
  }

  QOpenGLWidget::update();
}

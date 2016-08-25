#ifndef GLHEAPDIAGRAM_H
#define GLHEAPDIAGRAM_H

#include <memory>

#include <QOpenGLWidget>
#include <QOpenGLBuffer>
#include <QOpenGLVertexArrayObject>
#include <QOpenGLShaderProgram>
#include <QOpenGLFunctions>
#include <QPoint>

#include "heaphistory.h"
#include "transform3d.h"

class OpenGLShaderProgram;

class GLHeapDiagram : public QOpenGLWidget, protected QOpenGLFunctions {
public:
  explicit GLHeapDiagram(QWidget *parent = 0);
  ~GLHeapDiagram();
  QSize sizeHint();
  QSize minimumSizeHint();

signals:
  void frameSwapped();
  void blockClicked(bool, HeapBlock);
  void showMessage(std::string);

public slots:

protected slots:
  void update();

protected:
  void initializeGL();
  void paintGL();
  void resizeGL(int width, int height);

  void mousePressEvent(QMouseEvent *event);
  void mouseMoveEvent(QMouseEvent *event);
  void wheelEvent(QWheelEvent *event);

private:
  Q_OBJECT;
  // This should be a power-of-2.
  static constexpr uint32_t number_of_grid_lines_ = 16;

  // Initialization of the GL members below.
  void setupHeapblockGLStructures();
  void setupGridGLStructures();

  void updateHeapToScreenMap();
  void updateUnitSquareToHeapMap();

  void getScaleFromHeapToScreen(double* scale_x, double* scale_y);
  void getScaleFromScreenToHeap(double* scale_x, double* scale_y);
  bool screenToHeap(double, double, uint32_t* tick, uint64_t* address);
  //void heapToScreen(uint32_t tick, uint64_t address, double*, double*);

  void setHeapBaseUniforms();
  void setTickBaseUniforms();

  // Due to limited precision of floats, a straight mapping void setTickBaseUniforms();
  // heap to the screen space can have a degenerate matrix with a zero
  // entry. In order to prevent this, the scaling down is implemented
  // with two matrices that are applied iteratively.
  QMatrix2x2 heap_to_screen_matrix_;

  // VAO, VBO and shader for the heap blocks.
  QOpenGLBuffer heap_vertex_buffer_;
  QOpenGLVertexArrayObject heap_block_vao_;
  std::unique_ptr<QOpenGLShaderProgram> heap_shader_program_;
  // Stuff to map heap blocks to the screen.
  int uniform_vertex_to_screen_;
  int uniform_translation_part_;

  // An ivec3 that is filled with the uint64_t of the base address of the
  // displayed fraction of the heap.
  int uniform_visible_heap_base_A_;
  int uniform_visible_heap_base_B_;
  int uniform_visible_heap_base_C_;

  // The minimum tick is provided as ivec2.
  int uniform_visible_tick_base_A_;
  int uniform_visible_tick_base_B_;

  // VAO, VBO and shader for the grid.
  QOpenGLBuffer grid_vertex_buffer_;
  QOpenGLVertexArrayObject grid_vao_;
  std::unique_ptr<QOpenGLShaderProgram> grid_shader_;
  int uniform_grid_to_heap_map_;
  int uniform_heap_to_screen_map_;
  int uniform_grid_to_heap_translation_;

  // The heap history.
  HeapHistory heap_history_;

  // Last mouse position for dragging and selecting.
  QPoint last_mouse_position_;
  void setupUniformsForShaders();
  void debugDumpVerticesAndMappings();
};

#endif // GLHEAPDIAGRAM_H

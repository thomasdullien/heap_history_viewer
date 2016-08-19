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

  // Matrices and vectors for the mappings from heap space to screen
  // space, from the unit square to the heap space, the necessary
  // inverses.
  QMatrix2x2 unit_square_to_heap_matrix_;
  QVector2D unit_square_to_heap_translation_;
  // Due to limited precision of floats, a straight mapping from the
  // heap to the screen space can have a degenerate matrix with a zero
  // entry. In order to prevent this, the scaling down is implemented
  // with two matrices that are applied iteratively.
  QMatrix2x2 heap_to_4GB_matrix_;
  QMatrix2x2 heap_to_screen_matrix_;
  QVector2D heap_to_screen_translation_;
  // Since this mapping isn't going ot the GPU, it is more practical
  // to keep it as a QTransform.
  QTransform screen_to_heap_;

  // VAO, VBO and shader for the heap blocks.
  QOpenGLBuffer heap_vertex_buffer_;
  QOpenGLVertexArrayObject heap_block_vao_;
  std::unique_ptr<QOpenGLShaderProgram> heap_shader_program_;
  // Stuff to map heap blocks to the screen.
  int uniform_vertex_to_screen_;
  int uniform_translation_part_;
  QMatrix4x4 vertex_to_screen_map_;
  QVector4D translation_;

  // VAO, VBO and shader for the grid.
  QOpenGLBuffer grid_vertex_buffer_;
  QOpenGLVertexArrayObject grid_vao_;
  std::unique_ptr<QOpenGLShaderProgram> grid_shader_;
  int uniform_grid_to_heap_map_;
  int uniform_heap_to_screen_map_;
  int uniform_grid_to_heap_translation_;
  int uniform_heap_to_screen_translation_;

  // The heap history.
  HeapHistory heap_history_;

  // Last mouse position for dragging and selecting.
  QPoint last_mouse_position_;
};

#endif // GLHEAPDIAGRAM_H

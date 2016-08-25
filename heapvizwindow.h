#ifndef HEAPVIZWINDOW_H
#define HEAPVIZWINDOW_H

#include <QMainWindow>
#include <QKeyEvent>
#include <QStatusBar>

#include "heapblock.h"

namespace Ui {
class HeapVizWindow;
}

class HeapVizWindow : public QMainWindow {
  Q_OBJECT

public:
  explicit HeapVizWindow(QWidget *parent = 0);
  ~HeapVizWindow();

protected:
  void keyPressEvent(QKeyEvent *e);

public slots:
  void blockClicked(bool, HeapBlock);
  void showMessage(std::string);

protected slots:
  void update();

private slots:

private :
  Ui::HeapVizWindow *ui;
  QStatusBar statusbar_;
};

#endif // HEAPVIZWINDOW_H

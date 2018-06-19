#include "heapvizwindow.h"
#include <gflags/gflags.h>
#include <QApplication>
#include <QDesktopWidget>
#include <QSurfaceFormat>

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  
  std::string inputfile = "/tmp/heap.json";
  if (argc == 2) {
    inputfile = std::string(argv[1]);
  }

  HeapVizWindow w(&inputfile);

  w.setWindowTitle("Heap Visualisation in OpenGL");
  w.show();

  return a.exec();
}

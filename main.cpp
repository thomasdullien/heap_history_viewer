#include "heapvizwindow.h"
#include <gflags/gflags.h>
#include <QApplication>
#include <QDesktopWidget>
#include <QSurfaceFormat>

/*DEFINE_string(inputfile, "/tmp/heapdata.bin", "The file from which to read "
                                              "the heap event log.");
DEFINE_string(format, "json", "Specify the format of the input data: Either "
                              "JSON or binary.");
*/
int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  HeapVizWindow w;

  w.setWindowTitle("Heap Visualisation in OpenGL");
  w.show();

  return a.exec();
}

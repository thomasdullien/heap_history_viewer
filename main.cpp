#include "heapvizwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    HeapVizWindow w;
    w.show();

    return a.exec();
}

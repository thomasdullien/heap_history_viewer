#ifndef HEAPVIZWINDOW_H
#define HEAPVIZWINDOW_H

#include <QMainWindow>

namespace Ui {
class HeapVizWindow;
}

class HeapVizWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit HeapVizWindow(QWidget *parent = 0);
    ~HeapVizWindow();

private:
    Ui::HeapVizWindow *ui;
};

#endif // HEAPVIZWINDOW_H

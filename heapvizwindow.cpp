#include "heapvizwindow.h"
#include "ui_heapvizwindow.h"

HeapVizWindow::HeapVizWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::HeapVizWindow)
{
    ui->setupUi(this);
}

HeapVizWindow::~HeapVizWindow()
{
    delete ui;
}

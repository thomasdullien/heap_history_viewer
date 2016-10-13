#include "heapvizwindow.h"
#include "ui_heapvizwindow.h"
#include <QStatusBar>

HeapVizWindow::HeapVizWindow(const std::string* inputfile,
                             QWidget *parent)
    : QMainWindow(parent), ui(new Ui::HeapVizWindow) {
  ui->setupUi(this);

  statusBar()->showMessage("Initialized Main Window");

  if (inputfile != nullptr) {
    emit setFileToDisplay(QString(inputfile->c_str()));
  }
}

void HeapVizWindow::update() { printf("Update called"); }

HeapVizWindow::~HeapVizWindow() { delete ui; }

void HeapVizWindow::keyPressEvent(QKeyEvent *e) {
  if (e->key() == Qt::Key_Escape) {
    close();
  } else {
    QWidget::keyPressEvent(e);
  }
}

void HeapVizWindow::blockClicked(bool b, HeapBlock block) {
  statusBar()->showMessage( b ? getBlockInformationAsString(block).c_str() : "No block");
}

void HeapVizWindow::showMessage(std::string message) {
  QString message_to_show(message.c_str());
  statusBar()->showMessage(message_to_show);
}

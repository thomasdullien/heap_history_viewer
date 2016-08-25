#include "heapvizwindow.h"
#include "ui_heapvizwindow.h"
#include <QStatusBar>

HeapVizWindow::HeapVizWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::HeapVizWindow) {
  ui->setupUi(this);

  statusBar()->showMessage("Initialized Main Window");
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
  char buffer[255];
  if (b) {
    sprintf(buffer, "Block address: %lx Size: %lx (%d) Tick: %d", block.address_,
          block.size_, block.size_, block.start_tick_);
  } else {
    sprintf(buffer, "No block.");
  }
  statusBar()->showMessage(buffer);
}

void HeapVizWindow::showMessage(std::string message) {
  QString message_to_show(message.c_str());
  statusBar()->showMessage(message_to_show);
}

#include "heapvizwindow.h"
#include "ui_heapvizwindow.h"
#include <QFileDialog>
#include <QInputDialog>
#include <QStatusBar>

#include <istream>
#include <fstream>

HeapVizWindow::HeapVizWindow(const std::string* inputfile,
                             QWidget *parent)
    : QMainWindow(parent), ui(new Ui::HeapVizWindow) {
  ui->setupUi(this);

  statusBar()->showMessage("Initialized Main Window");

  bool can_file_be_opened = false;
  // Check if the input file can be opened.
  QString input_filename = QString::fromStdString(*inputfile);
  do {
    std::ifstream ifs(input_filename.toUtf8().constData(), std::fstream::in);
    if (ifs.fail()) {
      input_filename = QFileDialog::getOpenFileName(this, tr("Open Heap Log JSON"), "",
        tr("JSON Files (*.json)"));
    } else {
      can_file_be_opened = true;
    }
  } while (!can_file_be_opened);

  emit setFileToDisplay(input_filename);
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

void HeapVizWindow::on_actionHighlight_blocks_with_size_triggered()
{
  int size = QInputDialog::getInt(this, tr("Specify the size to highlight"),
    tr("Block size"), 256);

  emit setSizeToHighlight(size);
}

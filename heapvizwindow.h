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
  explicit HeapVizWindow(const std::string* inputfile = nullptr,
                         QWidget *parent = nullptr);
  ~HeapVizWindow() override;

protected:
  void keyPressEvent(QKeyEvent *e) override;

signals:
  void setFileToDisplay(QString filename);
  void setSizeToHighlight(uint32_t size);

public slots:
  void blockClicked(bool, HeapBlock);
  void showMessage(const std::string&);

protected slots:
  void update();

private slots:
  void on_actionHighlight_blocks_with_size_triggered();

private :
  Ui::HeapVizWindow *ui;
  QStatusBar statusbar_;
};

#endif // HEAPVIZWINDOW_H

/********************************************************************************
** Form generated from reading UI file 'heapvizwindow.ui'
**
** Created by: Qt User Interface Compiler version 5.2.1
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_HEAPVIZWINDOW_H
#define UI_HEAPVIZWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QToolBar>
#include <QtWidgets/QWidget>
#include "glheapdiagram.h"

QT_BEGIN_NAMESPACE

class Ui_HeapVizWindow
{
public:
    QAction *actionOpen_File;
    QAction *actionHighlight_blocks_with_size;
    QAction *actionReload_File;
    QWidget *centralWidget;
    QGridLayout *gridLayout;
    GLHeapDiagram *heap_diagram;
    QMenuBar *menuBar;
    QMenu *menuHeapViz_GL;
    QMenu *menuTest;
    QToolBar *mainToolBar;
    QStatusBar *statusBar;
    QToolBar *toolBar;

    void setupUi(QMainWindow *HeapVizWindow)
    {
        if (HeapVizWindow->objectName().isEmpty())
            HeapVizWindow->setObjectName(QStringLiteral("HeapVizWindow"));
        HeapVizWindow->resize(789, 801);
        actionOpen_File = new QAction(HeapVizWindow);
        actionOpen_File->setObjectName(QStringLiteral("actionOpen_File"));
        actionHighlight_blocks_with_size = new QAction(HeapVizWindow);
        actionHighlight_blocks_with_size->setObjectName(QStringLiteral("actionHighlight_blocks_with_size"));
        actionReload_File = new QAction(HeapVizWindow);
        actionReload_File->setObjectName(QStringLiteral("actionReload_File"));
        centralWidget = new QWidget(HeapVizWindow);
        centralWidget->setObjectName(QStringLiteral("centralWidget"));
        gridLayout = new QGridLayout(centralWidget);
        gridLayout->setSpacing(6);
        gridLayout->setContentsMargins(11, 11, 11, 11);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        heap_diagram = new GLHeapDiagram(centralWidget);
        heap_diagram->setObjectName(QStringLiteral("heap_diagram"));
        heap_diagram->setMouseTracking(true);

        gridLayout->addWidget(heap_diagram, 0, 0, 1, 1);

        HeapVizWindow->setCentralWidget(centralWidget);
        menuBar = new QMenuBar(HeapVizWindow);
        menuBar->setObjectName(QStringLiteral("menuBar"));
        menuBar->setGeometry(QRect(0, 0, 789, 28));
        menuHeapViz_GL = new QMenu(menuBar);
        menuHeapViz_GL->setObjectName(QStringLiteral("menuHeapViz_GL"));
        menuTest = new QMenu(menuBar);
        menuTest->setObjectName(QStringLiteral("menuTest"));
        HeapVizWindow->setMenuBar(menuBar);
        mainToolBar = new QToolBar(HeapVizWindow);
        mainToolBar->setObjectName(QStringLiteral("mainToolBar"));
        HeapVizWindow->addToolBar(Qt::TopToolBarArea, mainToolBar);
        statusBar = new QStatusBar(HeapVizWindow);
        statusBar->setObjectName(QStringLiteral("statusBar"));
        HeapVizWindow->setStatusBar(statusBar);
        toolBar = new QToolBar(HeapVizWindow);
        toolBar->setObjectName(QStringLiteral("toolBar"));
        HeapVizWindow->addToolBar(Qt::TopToolBarArea, toolBar);

        menuBar->addAction(menuHeapViz_GL->menuAction());
        menuBar->addAction(menuTest->menuAction());
        menuHeapViz_GL->addAction(actionOpen_File);
        menuHeapViz_GL->addAction(actionReload_File);
        menuTest->addAction(actionHighlight_blocks_with_size);

        retranslateUi(HeapVizWindow);
        QObject::connect(heap_diagram, SIGNAL(blockClicked(bool,HeapBlock)), HeapVizWindow, SLOT(blockClicked(bool,HeapBlock)));
        QObject::connect(heap_diagram, SIGNAL(showMessage(std::string)), HeapVizWindow, SLOT(showMessage(std::string)));
        QObject::connect(HeapVizWindow, SIGNAL(setFileToDisplay(QString)), heap_diagram, SLOT(setFileToDisplay(QString)));

        QMetaObject::connectSlotsByName(HeapVizWindow);
    } // setupUi

    void retranslateUi(QMainWindow *HeapVizWindow)
    {
        HeapVizWindow->setWindowTitle(QApplication::translate("HeapVizWindow", "HeapVizWindow", 0));
        actionOpen_File->setText(QApplication::translate("HeapVizWindow", "Load File", 0));
        actionHighlight_blocks_with_size->setText(QApplication::translate("HeapVizWindow", "Highlight blocks in size range", 0));
        actionReload_File->setText(QApplication::translate("HeapVizWindow", "Reload File", 0));
        menuHeapViz_GL->setTitle(QApplication::translate("HeapVizWindow", "HeapViz GL", 0));
        menuTest->setTitle(QApplication::translate("HeapVizWindow", "Edit", 0));
        toolBar->setWindowTitle(QApplication::translate("HeapVizWindow", "toolBar", 0));
    } // retranslateUi

};

namespace Ui {
    class HeapVizWindow: public Ui_HeapVizWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_HEAPVIZWINDOW_H

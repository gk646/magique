#include "MainWindow.h"

#include <QApplication>
#include <QDir>
#include <QDockWidget>
#include <QFileSystemModel>
#include <QMainWindow>
#include <QMenuBar>
#include <QStatusBar>
#include <QTreeView>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent)
{
    // Menu Bar
    auto *menuBar = new QMenuBar();
    QMenu *fileMenu = menuBar->addMenu(tr("&File"));
    QMenu *editMenu = menuBar->addMenu(tr("&Edit"));
    QMenu *viewMenu = menuBar->addMenu(tr("&View"));
    setMenuBar(menuBar);

    // File Menu Actions
    fileMenu->addAction("Open", this, nullptr);
    fileMenu->addAction("Exit", this, &QApplication::quit);

    // Setup File System Model
    auto *fileSystemModel = new QFileSystemModel(this);
    fileSystemModel->setRootPath(QDir::homePath());

    // Setup File Tree View
    auto *treeView = new QTreeView();
    treeView->setModel(fileSystemModel);
    treeView->setRootIndex(fileSystemModel->index(QDir::homePath()));

    // Dock Widget for File Tree
    auto *dockWidget = new QDockWidget("File Reader", this);
    dockWidget->setWidget(treeView);
    addDockWidget(Qt::LeftDockWidgetArea, dockWidget);

    dockWidget->setFeatures(QDockWidget::DockWidgetMovable | QDockWidget::DockWidgetFloatable);

    auto *centralWidget = new QWidget();
    setCentralWidget(centralWidget);

    // Setup StatusBar
    statusBar()->showMessage(tr("Ready"));
}

 MainWindow::~MainWindow()
{

}
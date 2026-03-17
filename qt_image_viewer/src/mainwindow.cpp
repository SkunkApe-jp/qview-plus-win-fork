#include "mainwindow.h"
#include "imageviewer.h"
#include "imagemodel.h"

#include <QToolBar>
#include <QAction>
#include <QFileDialog>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , viewer(new ImageViewer(this))
{
    setCentralWidget(viewer);
    resize(800, 600);
    setWindowTitle("Qt Image Viewer");

    createActions();
    createToolBar();
}

MainWindow::~MainWindow()
{
}

void MainWindow::createActions()
{
    openAct = new QAction(tr("&Open..."), this);
    openAct->setShortcut(QKeySequence::Open);
    connect(openAct, &QAction::triggered, viewer->model(), &ImageModel::open);

    zoomInAct = new QAction(tr("Zoom &In"), this);
    zoomInAct->setShortcut(QKeySequence::ZoomIn);
    connect(zoomInAct, &QAction::triggered, viewer, &ImageViewer::zoomIn);

    zoomOutAct = new QAction(tr("Zoom &Out"), this);
    zoomOutAct->setShortcut(QKeySequence::ZoomOut);
    connect(zoomOutAct, &QAction::triggered, viewer, &ImageViewer::zoomOut);

    fitToWindowAct = new QAction(tr("&Fit to Window"), this);
    fitToWindowAct->setCheckable(true);
    connect(fitToWindowAct, &QAction::triggered, viewer, &ImageViewer::fitToWindow);

    prevAct = new QAction(tr("&Previous"), this);
    prevAct->setShortcut(QKeySequence::MoveToPreviousPage);
    connect(prevAct, &QAction::triggered, viewer->model(), &ImageModel::previousImage);

    nextAct = new QAction(tr("&Next"), this);
    nextAct->setShortcut(QKeySequence::MoveToNextPage);
    connect(nextAct, &QAction::triggered, viewer->model(), &ImageModel::nextImage);

    rotateLeftAct = new QAction(tr("Rotate &Left"), this);
    connect(rotateLeftAct, &QAction::triggered, viewer->model(), &ImageModel::rotateLeft);

    rotateRightAct = new QAction(tr("Rotate &Right"), this);
    connect(rotateRightAct, &QAction::triggered, viewer->model(), &ImageModel::rotateRight);
    
    flipAct = new QAction(tr("Flip &Horizontal"), this);
    connect(flipAct, &QAction::triggered, viewer->model(), &ImageModel::flipHorizontal);
}

void MainWindow::createToolBar()
{
    toolBar = addToolBar(tr("Main Toolbar"));
    toolBar->addAction(openAct);
    toolBar->addSeparator();
    toolBar->addAction(prevAct);
    toolBar->addAction(nextAct);
    toolBar->addSeparator();
    toolBar->addAction(zoomInAct);
    toolBar->addAction(zoomOutAct);
    toolBar->addAction(fitToWindowAct);
    toolBar->addSeparator();
    toolBar->addAction(rotateLeftAct);
    toolBar->addAction(rotateRightAct);
    toolBar->addAction(flipAct);
}

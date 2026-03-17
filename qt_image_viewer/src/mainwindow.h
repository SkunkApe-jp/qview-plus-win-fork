#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class ImageViewer;
class QToolBar;
class QAction;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

private:
    void createActions();
    void createToolBar();

    ImageViewer *viewer;

    QAction *openAct;
    QAction *zoomInAct;
    QAction *zoomOutAct;
    QAction *fitToWindowAct;
    QAction *prevAct;
    QAction *nextAct;
    QAction *rotateRightAct;
    QAction *rotateLeftAct;
    QAction *flipAct;

    QToolBar *toolBar;
};

#endif // MAINWINDOW_H

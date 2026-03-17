#ifndef IMAGEVIEWER_H
#define IMAGEVIEWER_H

#include <QGraphicsView>

class ImageModel;
class QGraphicsScene;
class QGraphicsPixmapItem;

class ImageViewer : public QGraphicsView
{
    Q_OBJECT

public:
    explicit ImageViewer(QWidget *parent = nullptr);
    ImageModel* model() const { return m_model; }

public slots:
    void zoomIn();
    void zoomOut();
    void fitToWindow(bool fit);

protected:
    void wheelEvent(QWheelEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void onImageLoaded(const QPixmap &pixmap);

private:
    void scaleImage(double factor, const QPoint &pos);

    ImageModel *m_model;
    QGraphicsScene *m_scene;
    QGraphicsPixmapItem *m_pixmapItem;

    bool m_fitToWindow;

    // Pan state
    bool m_panning;
    QPoint m_lastPanPoint;
};

#endif // IMAGEVIEWER_H

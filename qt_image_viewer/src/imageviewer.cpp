#include "imageviewer.h"
#include "imagemodel.h"

#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QWheelEvent>
#include <QMouseEvent>
#include <QScrollBar>
#include <qmath.h>

ImageViewer::ImageViewer(QWidget *parent)
    : QGraphicsView(parent)
    , m_model(new ImageModel(this))
    , m_scene(new QGraphicsScene(this))
    , m_pixmapItem(nullptr)
    , m_fitToWindow(false)
    , m_panning(false)
{
    setScene(m_scene);
    setRenderHint(QPainter::Antialiasing);
    setRenderHint(QPainter::SmoothPixmapTransform);
    setDragMode(QGraphicsView::NoDrag); // We'll implement custom panning
    
    // Hide scrollbars for a cleaner look
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    connect(m_model, &ImageModel::imageLoaded, this, &ImageViewer::onImageLoaded);
}

void ImageViewer::onImageLoaded(const QPixmap &pixmap)
{
    m_scene->clear();
    m_pixmapItem = m_scene->addPixmap(pixmap);
    m_pixmapItem->setTransformationMode(Qt::SmoothTransformation);
    
    // Adjust scene rect to match the image dimensions
    m_scene->setSceneRect(m_pixmapItem->boundingRect());

    if (m_fitToWindow) {
        fitToWindow(true);
    } else {
        resetTransform();
        centerOn(m_pixmapItem);
    }
}

void ImageViewer::zoomIn()
{
    scaleImage(1.2, viewport()->rect().center());
}

void ImageViewer::zoomOut()
{
    scaleImage(1.0 / 1.2, viewport()->rect().center());
}

void ImageViewer::fitToWindow(bool fit)
{
    m_fitToWindow = fit;
    if (fit && m_pixmapItem) {
        resetTransform();
        fitInView(m_pixmapItem, Qt::KeepAspectRatio);
    }
}

void ImageViewer::scaleImage(double factor, const QPoint &pos)
{
    if (!m_pixmapItem) return;

    if (m_fitToWindow) {
        m_fitToWindow = false; // Disable fit to window if user manually zooms
    }

    // Capture the scene position before scaling
    QPointF scenePosBefore = mapToScene(pos);
    
    // Scale the view
    scale(factor, factor);

    // Capture the scene position after scaling
    QPointF scenePosAfter = mapToScene(pos);

    // Translate the view to keep the point under the mouse consistent
    QPointF diff = scenePosAfter - scenePosBefore;
    translate(diff.x(), diff.y());
}

void ImageViewer::wheelEvent(QWheelEvent *event)
{
    if (event->modifiers() & Qt::ControlModifier) {
        // Zoom with Ctrl+Wheel
        double scaleFactor = qPow(1.0015, event->angleDelta().y());
        scaleImage(scaleFactor, event->position().toPoint());
        event->accept();
    } else {
        // Default scroll behavior (or we can just zoom normally without Ctrl like many viewers)
        double scaleFactor = qPow(1.0015, event->angleDelta().y());
        scaleImage(scaleFactor, event->position().toPoint());
        event->accept();
    }
}

void ImageViewer::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        m_panning = true;
        m_lastPanPoint = event->pos();
        setCursor(Qt::ClosedHandCursor);
        event->accept();
    } else {
        QGraphicsView::mousePressEvent(event);
    }
}

void ImageViewer::mouseMoveEvent(QMouseEvent *event)
{
    if (m_panning) {
        QPoint delta = event->pos() - m_lastPanPoint;
        m_lastPanPoint = event->pos();
        
        // Disable scrollbars to avoid weird behavior, translate the view manually
        QPointF sceneDelta = mapToScene(0, 0) - mapToScene(delta.x(), delta.y());
        translate(-sceneDelta.x() * transform().m11(), -sceneDelta.y() * transform().m22());
        
        event->accept();
    } else {
        QGraphicsView::mouseMoveEvent(event);
    }
}

void ImageViewer::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && m_panning) {
        m_panning = false;
        setCursor(Qt::ArrowCursor);
        event->accept();
    } else {
        QGraphicsView::mouseReleaseEvent(event);
    }
}

void ImageViewer::resizeEvent(QResizeEvent *event)
{
    QGraphicsView::resizeEvent(event);
    if (m_fitToWindow) {
        fitToWindow(true);
    }
}

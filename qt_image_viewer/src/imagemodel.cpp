#include "imagemodel.h"

#include <QDir>
#include <QFileInfo>
#include <QImageReader>
#include <QFileDialog>

ImageModel::ImageModel(QObject *parent)
    : QObject(parent), m_currentIndex(-1), m_rotationAngle(0), m_flippedHorizontal(false)
{
}

void ImageModel::open()
{
    QString fileName = QFileDialog::getOpenFileName(nullptr, tr("Open File"),
                                                    QDir::currentPath(),
                                                    tr("Images (*.png *.xpm *.jpg *.jpeg *.bmp *.gif)"));
    if (!fileName.isEmpty())
        loadFile(fileName);
}

bool ImageModel::loadFile(const QString &fileName)
{
    QImageReader reader(fileName);
    reader.setAutoTransform(true);
    QImage newImage = reader.read();
    if (newImage.isNull()) {
        emit message(tr("Cannot load %1: %2")
                         .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        return false;
    }

    m_currentFile = fileName;
    m_originalPixmap = QPixmap::fromImage(newImage);
    
    // Reset transformations on new image load
    m_rotationAngle = 0;
    m_flippedHorizontal = false;

    updateDirectoryList(fileName);
    
    // Initial emit
    applyTransformations();

    emit message(tr("Loaded %1").arg(QDir::toNativeSeparators(fileName)));
    return true;
}

void ImageModel::updateDirectoryList(const QString &fileName)
{
    QFileInfo fileInfo(fileName);
    QDir dir = fileInfo.dir();

    QStringList nameFilters;
    nameFilters << "*.png" << "*.xpm" << "*.jpg" << "*.jpeg" << "*.bmp" << "*.gif";
    
    m_directoryFiles = dir.entryList(nameFilters, QDir::Files | QDir::Readable, QDir::Name);
    
    for (int i = 0; i < m_directoryFiles.size(); ++i) {
        m_directoryFiles[i] = dir.absoluteFilePath(m_directoryFiles[i]);
    }

    m_currentIndex = m_directoryFiles.indexOf(fileInfo.absoluteFilePath());
}

void ImageModel::nextImage()
{
    if (m_directoryFiles.isEmpty()) return;

    m_currentIndex++;
    if (m_currentIndex >= m_directoryFiles.size()) {
        m_currentIndex = 0; // Wrap around
    }

    loadFile(m_directoryFiles.at(m_currentIndex));
}

void ImageModel::previousImage()
{
    if (m_directoryFiles.isEmpty()) return;

    m_currentIndex--;
    if (m_currentIndex < 0) {
        m_currentIndex = m_directoryFiles.size() - 1; // Wrap around
    }

    loadFile(m_directoryFiles.at(m_currentIndex));
}

void ImageModel::rotateLeft()
{
    if (m_originalPixmap.isNull()) return;
    m_rotationAngle = (m_rotationAngle - 90) % 360;
    if (m_rotationAngle < 0) m_rotationAngle += 360;
    applyTransformations();
}

void ImageModel::rotateRight()
{
    if (m_originalPixmap.isNull()) return;
    m_rotationAngle = (m_rotationAngle + 90) % 360;
    applyTransformations();
}

void ImageModel::flipHorizontal()
{
    if (m_originalPixmap.isNull()) return;
    m_flippedHorizontal = !m_flippedHorizontal;
    applyTransformations();
}

void ImageModel::applyTransformations()
{
    if (m_originalPixmap.isNull()) return;

    QTransform transform;
    if (m_flippedHorizontal) {
        transform.scale(-1, 1);
    }
    transform.rotate(m_rotationAngle);
    
    m_pixmap = m_originalPixmap.transformed(transform, Qt::SmoothTransformation);
    emit imageLoaded(m_pixmap);
}

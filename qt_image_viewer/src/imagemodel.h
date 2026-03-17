#ifndef IMAGEMODEL_H
#define IMAGEMODEL_H

#include <QObject>
#include <QPixmap>
#include <QStringList>

class ImageModel : public QObject
{
    Q_OBJECT

public:
    explicit ImageModel(QObject *parent = nullptr);

    bool loadFile(const QString &fileName);
    QString currentFileName() const { return m_currentFile; }
    QPixmap currentPixmap() const { return m_pixmap; }

public slots:
    void open();
    void nextImage();
    void previousImage();
    void rotateLeft();
    void rotateRight();
    void flipHorizontal();

signals:
    void imageLoaded(const QPixmap &pixmap);
    void message(const QString &msg);

private:
    void updateDirectoryList(const QString &fileName);
    void applyTransformations();

    QString m_currentFile;
    QPixmap m_pixmap;
    
    // Original unmodified pixmap
    QPixmap m_originalPixmap;

    QStringList m_directoryFiles;
    int m_currentIndex;

    int m_rotationAngle;
    bool m_flippedHorizontal;
};

#endif // IMAGEMODEL_H

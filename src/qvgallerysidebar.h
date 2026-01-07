#ifndef QVGALLERYSIDEBAR_H
#define QVGALLERYSIDEBAR_H

#include <QWidget>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include "qvimagecore.h"

class QVGallerySidebar : public QWidget
{
    Q_OBJECT

public:
    explicit QVGallerySidebar(QWidget *parent = nullptr);
    void loadGallery(const QList<QVImageCore::CompatibleFile> &imageFiles, int currentIndex);

signals:
    void imageSelected(const QString &filePath);

private slots:
    void onThumbnailClicked(QListWidgetItem *item);
    void onRefreshClicked();

private:
    void createThumbnail(const QVImageCore::CompatibleFile &file, bool isCurrent);

    QListWidget *thumbnailList;
    QPushButton *refreshButton;
    QLabel *countLabel;
    QList<QVImageCore::CompatibleFile> currentImageFiles;
    int currentImageIndex;
};

#endif // QVGALLERYSIDEBAR_H

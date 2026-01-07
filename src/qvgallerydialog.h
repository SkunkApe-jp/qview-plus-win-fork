#ifndef QVGALLERYDIALOG_H
#define QVGALLERYDIALOG_H

#include <QDialog>
#include <QListWidget>
#include <QPushButton>
#include <QLabel>
#include "qvimagecore.h"

class QVGalleryDialog : public QDialog
{
    Q_OBJECT

public:
    explicit QVGalleryDialog(QWidget *parent = nullptr);
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

#endif // QVGALLERYDIALOG_H

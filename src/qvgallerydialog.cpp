#include "qvgallerydialog.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QImageReader>
#include <QFileIconProvider>

QVGalleryDialog::QVGalleryDialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("Gallery View"));
    setMinimumSize(700, 500);
    
    auto *layout = new QVBoxLayout(this);
    
    // Info bar
    auto *infoLayout = new QHBoxLayout();
    countLabel = new QLabel(this);
    refreshButton = new QPushButton(tr("Refresh"), this);
    connect(refreshButton, &QPushButton::clicked, this, &QVGalleryDialog::onRefreshClicked);
    
    infoLayout->addWidget(countLabel);
    infoLayout->addStretch();
    infoLayout->addWidget(refreshButton);
    layout->addLayout(infoLayout);
    
    // Thumbnail grid
    thumbnailList = new QListWidget(this);
    thumbnailList->setViewMode(QListWidget::IconMode);
    thumbnailList->setIconSize(QSize(180, 180));
    thumbnailList->setSpacing(12);
    thumbnailList->setResizeMode(QListWidget::Adjust);
    thumbnailList->setMovement(QListWidget::Static);
    thumbnailList->setWrapping(true);
    connect(thumbnailList, &QListWidget::itemClicked, this, &QVGalleryDialog::onThumbnailClicked);
    connect(thumbnailList, &QListWidget::itemDoubleClicked, this, &QVGalleryDialog::onThumbnailClicked);
    
    layout->addWidget(thumbnailList);
    
    // Bottom buttons
    auto *buttonLayout = new QHBoxLayout();
    auto *closeButton = new QPushButton(tr("Close"), this);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);
    
    buttonLayout->addStretch();
    buttonLayout->addWidget(closeButton);
    layout->addLayout(buttonLayout);
}

void QVGalleryDialog::loadGallery(const QList<QVImageCore::CompatibleFile> &imageFiles, int currentIndex)
{
    thumbnailList->clear();
    currentImageFiles = imageFiles;
    currentImageIndex = currentIndex;
    
    countLabel->setText(tr("%1 images in folder").arg(imageFiles.count()));
    
    for (int i = 0; i < imageFiles.count(); ++i) {
        createThumbnail(imageFiles.at(i), i == currentIndex);
    }
    
    // Scroll to current image
    if (currentIndex >= 0 && currentIndex < thumbnailList->count()) {
        thumbnailList->setCurrentRow(currentIndex);
        thumbnailList->scrollToItem(thumbnailList->item(currentIndex));
    }
}

void QVGalleryDialog::createThumbnail(const QVImageCore::CompatibleFile &file, bool isCurrent)
{
    QImageReader reader(file.absoluteFilePath);
    reader.setAutoTransform(true);
    
    // Scale image to thumbnail size
    QSize imageSize = reader.size();
    if (imageSize.isValid()) {
        if (imageSize.width() > 180 || imageSize.height() > 180) {
            imageSize.scale(QSize(180, 180), Qt::KeepAspectRatio);
            reader.setScaledSize(imageSize);
        }
    }
    
    QImage thumbnail = reader.read();
    QPixmap pixmap;
    
    if (!thumbnail.isNull()) {
        pixmap = QPixmap::fromImage(thumbnail);
    } else {
        // Fallback icon if image can't be read
        pixmap = QPixmap(180, 180);
        pixmap.fill(Qt::lightGray);
    }
    
    auto *item = new QListWidgetItem(thumbnailList);
    item->setIcon(QIcon(pixmap));
    item->setText(file.fileName);
    item->setData(Qt::UserRole, file.absoluteFilePath);
    
    // Highlight current image
    if (isCurrent) {
        item->setBackground(QBrush(QColor(220, 240, 255)));
        QFont font = item->font();
        font.setBold(true);
        item->setFont(font);
    }
    
    thumbnailList->addItem(item);
}

void QVGalleryDialog::onThumbnailClicked(QListWidgetItem *item)
{
    QString filePath = item->data(Qt::UserRole).toString();
    emit imageSelected(filePath);
    accept();
}

void QVGalleryDialog::onRefreshClicked()
{
    loadGallery(currentImageFiles, currentImageIndex);
}

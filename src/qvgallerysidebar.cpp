#include "qvgallerysidebar.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPixmap>
#include <QImageReader>
#include <QFileIconProvider>

QVGallerySidebar::QVGallerySidebar(QWidget *parent) : QWidget(parent)
{
    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(5, 5, 5, 5);
    
    // Info bar
    auto *infoLayout = new QHBoxLayout();
    QLabel *titleLabel = new QLabel(tr("Gallery"), this);
    titleLabel->setStyleSheet("font-size: 12px; font-weight: bold; color: white;");
    
    countLabel = new QLabel(this);
    countLabel->setStyleSheet("font-size: 11px; color: #888; margin-left: 5px;");
    
    refreshButton = new QPushButton(tr("Refresh"), this);
    refreshButton->setFlat(true);
    refreshButton->setMaximumWidth(60);
    refreshButton->setCursor(Qt::PointingHandCursor);
    refreshButton->setStyleSheet("QPushButton { color: #eee; border: none; } QPushButton:hover { color: white; background: rgba(255,255,255,0.1); border-radius: 4px; }");
    connect(refreshButton, &QPushButton::clicked, this, &QVGallerySidebar::onRefreshClicked);
    
    infoLayout->addWidget(titleLabel);
    infoLayout->addWidget(countLabel);
    infoLayout->addStretch();
    infoLayout->addWidget(refreshButton);
    layout->addLayout(infoLayout);
    
    // Thumbnail grid
    thumbnailList = new QListWidget(this);
    thumbnailList->setViewMode(QListWidget::IconMode);
    thumbnailList->setIconSize(QSize(120, 120)); // Slightly smaller for sidebar
    thumbnailList->setSpacing(8);
    thumbnailList->setResizeMode(QListWidget::Adjust);
    thumbnailList->setMovement(QListWidget::Static);
    thumbnailList->setWrapping(true);
    thumbnailList->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    thumbnailList->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    thumbnailList->setStyleSheet("QListWidget { border: none; background: transparent; color: white; } "
                               "QListWidget::item { color: #eee; } "
                               "QListWidget::item:selected { background: rgba(0, 120, 215, 60); color: white; } "
                               "QListWidget::item:hover { background: rgba(255, 255, 255, 20); }");
    
    connect(thumbnailList, &QListWidget::itemClicked, this, &QVGallerySidebar::onThumbnailClicked);
    connect(thumbnailList, &QListWidget::itemDoubleClicked, this, &QVGallerySidebar::onThumbnailClicked);
    
    layout->addWidget(thumbnailList);
}

void QVGallerySidebar::loadGallery(const QList<QVImageCore::CompatibleFile> &imageFiles, int currentIndex)
{
    thumbnailList->clear();
    currentImageFiles = imageFiles;
    currentImageIndex = currentIndex;
    
    countLabel->setText(tr("%1 images").arg(imageFiles.count()));
    
    for (int i = 0; i < imageFiles.count(); ++i) {
        createThumbnail(imageFiles.at(i), i == currentIndex);
    }
    
    // Scroll to current image
    if (currentIndex >= 0 && currentIndex < thumbnailList->count()) {
        thumbnailList->setCurrentRow(currentIndex);
        thumbnailList->scrollToItem(thumbnailList->item(currentIndex));
    }
}

void QVGallerySidebar::createThumbnail(const QVImageCore::CompatibleFile &file, bool isCurrent)
{
    QImageReader reader(file.absoluteFilePath);
    reader.setAutoTransform(true);
    
    // Scale image to thumbnail size
    QSize imageSize = reader.size();
    if (imageSize.isValid()) {
        if (imageSize.width() > 120 || imageSize.height() > 120) {
            imageSize.scale(QSize(120, 120), Qt::KeepAspectRatio);
            reader.setScaledSize(imageSize);
        }
    }
    
    QImage thumbnail = reader.read();
    QPixmap pixmap;
    
    if (!thumbnail.isNull()) {
        pixmap = QPixmap::fromImage(thumbnail);
    } else {
        pixmap = QPixmap(120, 120);
        pixmap.fill(Qt::lightGray);
    }
    
    auto *item = new QListWidgetItem(thumbnailList);
    item->setIcon(QIcon(pixmap));
    item->setText(file.fileName);
    item->setData(Qt::UserRole, file.absoluteFilePath);
    item->setSizeHint(QSize(130, 150));
    item->setTextAlignment(Qt::AlignCenter);
    
    // Highlight current image
    if (isCurrent) {
        item->setBackground(QBrush(QColor(0, 120, 215, 40)));
        QFont font = item->font();
        font.setBold(true);
        item->setFont(font);
    }
    
    thumbnailList->addItem(item);
}

void QVGallerySidebar::onThumbnailClicked(QListWidgetItem *item)
{
    QString filePath = item->data(Qt::UserRole).toString();
    emit imageSelected(filePath);
}

void QVGallerySidebar::onRefreshClicked()
{
    loadGallery(currentImageFiles, currentImageIndex);
}

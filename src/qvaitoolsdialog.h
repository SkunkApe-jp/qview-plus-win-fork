#ifndef QVAITOOLSDIALOG_H
#define QVAITOOLSDIALOG_H

#include <QDialog>
#include <QFileInfo>
#include <QTextBrowser>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class QVAIToolsDialog : public QDialog
{
    Q_OBJECT

public:
    enum Mode {
        Description,
        OCR
    };

    QVAIToolsDialog(QWidget *parent, QFileInfo fileInfo, Mode mode);

private slots:
    void onNetworkReply(QNetworkReply *reply);
    void onCopyClicked();

private:
    void performAIAction();
    QString markdownToHtml(const QString &markdown);
    QString processMath(const QString &text);

    QFileInfo fileInfo;
    Mode mode;
    QTextBrowser *resultBrowser;
    QLabel *statusLabel;
    QProgressBar *progressBar;
    QPushButton *copyButton;
    QPushButton *closeButton;
    QNetworkAccessManager *networkManager;
};

#endif // QVAITOOLSDIALOG_H

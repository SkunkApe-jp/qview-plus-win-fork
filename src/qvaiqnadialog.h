#ifndef QVAIQNADIALOG_H
#define QVAIQNADIALOG_H

#include <QDialog>
#include <QFileInfo>
#include <QTextBrowser>
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>
#include <QProgressBar>
#include <QNetworkAccessManager>
#include <QNetworkReply>

class QVAIQnADialog : public QDialog
{
    Q_OBJECT

public:
    explicit QVAIQnADialog(QWidget *parent = nullptr);
    void setImage(const QFileInfo &fileInfo);

private slots:
    void onSendClicked();
    void onNetworkReply(QNetworkReply *reply);
    void onClearClicked();

private:
    void appendMessage(const QString &role, const QString &text, bool isMarkdown = false);
    void sendQuery(const QString &query);
    QString markdownToHtml(const QString &markdown);
    QString processMath(const QString &text);

    QFileInfo currentFileInfo;
    QTextBrowser *chatHistory;
    QLineEdit *inputField;
    QPushButton *sendButton;
    QPushButton *clearButton;
    QProgressBar *progressBar;
    QNetworkAccessManager *networkManager;
    QString chatHtml;
};

#endif // QVAIQNADIALOG_H

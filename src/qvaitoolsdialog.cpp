#include "qvaitoolsdialog.h"
#include "qvapplication.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QClipboard>
#include <QRegularExpression>

QVAIToolsDialog::QVAIToolsDialog(QWidget *parent, QFileInfo fileInfo, Mode mode)
    : QDialog(parent), fileInfo(fileInfo), mode(mode)
{
    setWindowTitle(mode == Description ? tr("AI Image Description") : tr("AI Text Extraction (OCR)"));
    setMinimumSize(500, 400);

    auto *layout = new QVBoxLayout(this);

    statusLabel = new QLabel(tr("Processing image..."), this);
    layout->addWidget(statusLabel);

    progressBar = new QProgressBar(this);
    progressBar->setMaximum(0);
    layout->addWidget(progressBar);

    resultBrowser = new QTextBrowser(this);
    resultBrowser->setReadOnly(true);
    resultBrowser->setOpenExternalLinks(true);
    // Apply basic styling similar to QnA
    resultBrowser->setHtml(R"(
<!DOCTYPE html>
<html>
<head>
<style>
body { font-family: system-ui, -apple-system, sans-serif; margin: 8px; line-height: 1.5; color: #333; }
code { background: #f0f0f0; padding: 2px 4px; border-radius: 3px; font-family: monospace; font-size: 0.95em; }
pre { background: #f5f5f5; border: 1px solid #ddd; padding: 10px; border-radius: 4px; overflow-x: auto; margin: 8px 0; }
pre code { background: none; padding: 0; }
blockquote { border-left: 3px solid #ccc; margin: 8px 0; padding-left: 10px; color: #555; }
ul, ol { margin: 4px 0; padding-left: 20px; }
h1, h2, h3 { margin: 10px 0 6px 0; }
h1 { font-size: 1.4em; }
h2 { font-size: 1.2em; }
h3 { font-size: 1.1em; }
</style>
</head>
<body>
</body>
</html>
)");
    layout->addWidget(resultBrowser);

    auto *buttonLayout = new QHBoxLayout();
    copyButton = new QPushButton(tr("Copy to Clipboard"), this);
    copyButton->setEnabled(false);
    connect(copyButton, &QPushButton::clicked, this, &QVAIToolsDialog::onCopyClicked);

    closeButton = new QPushButton(tr("Close"), this);
    connect(closeButton, &QPushButton::clicked, this, &QDialog::accept);

    buttonLayout->addStretch();
    buttonLayout->addWidget(copyButton);
    buttonLayout->addWidget(closeButton);
    layout->addLayout(buttonLayout);

    networkManager = new QNetworkAccessManager(this);
    performAIAction();
}

void QVAIToolsDialog::performAIAction()
{
    QString apiKey = qEnvironmentVariable("QVIEW_AI_API_KEY", "");
    if (apiKey.isEmpty()) {
        apiKey = qvApp->getSettingsManager().getString("airename/apikey");
    }

    if (apiKey.isEmpty()) {
        statusLabel->setText(tr("Error: API key not configured."));
        progressBar->setVisible(false);
        return;
    }

    QString baseUrl = qEnvironmentVariable("QVIEW_AI_BASE_URL",
        qvApp->getSettingsManager().getString("airename/baseurl", false).isEmpty() ? 
        "https://api.openai.com/v1" : 
        qvApp->getSettingsManager().getString("airename/baseurl", false));
    QString modelName = qEnvironmentVariable("QVIEW_AI_MODEL",
        qvApp->getSettingsManager().getString("airename/model", false).isEmpty() ? 
        "gpt-4-vision-preview" : 
        qvApp->getSettingsManager().getString("airename/model", false));

    QFile imageFile(fileInfo.absoluteFilePath());
    if (!imageFile.open(QIODevice::ReadOnly)) {
        statusLabel->setText(tr("Error: Could not read image file."));
        progressBar->setVisible(false);
        return;
    }
    QByteArray base64Image = imageFile.readAll().toBase64();
    imageFile.close();

    QString endpoint = baseUrl;
    if (!endpoint.endsWith("/chat/completions")) {
        if (!endpoint.endsWith("/")) endpoint += "/";
        endpoint += "chat/completions";
    }

    QNetworkRequest request{QUrl(endpoint)};
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    request.setRawHeader("Authorization", QString("Bearer %1").arg(apiKey).toUtf8());

    QJsonObject payload;
    payload["model"] = modelName;

    QJsonArray messages;
    QJsonObject message;
    message["role"] = "user";

    QJsonArray contentArray;
    QJsonObject textContent;
    textContent["type"] = "text";
    textContent["text"] = (mode == Description) ? 
        "Please describe this image in detail. Use markdown formatting where appropriate." : 
        "Extract all text from this image using OCR and return it. Preserve formatting with markdown if possible.";
    contentArray.append(textContent);

    QJsonObject imageContent;
    imageContent["type"] = "image_url";
    QJsonObject imageUrlObj;
    imageUrlObj["url"] = QString("data:image/jpeg;base64,%1").arg(QString::fromLatin1(base64Image));
    imageContent["image_url"] = imageUrlObj;
    contentArray.append(imageContent);

    message["content"] = contentArray;
    messages.append(message);
    payload["messages"] = messages;

    connect(networkManager, &QNetworkAccessManager::finished, this, &QVAIToolsDialog::onNetworkReply);
    networkManager->post(request, QJsonDocument(payload).toJson());
}

void QVAIToolsDialog::onNetworkReply(QNetworkReply *reply)
{
    progressBar->setVisible(false);
    if (reply->error() != QNetworkReply::NoError) {
        statusLabel->setText(tr("Error: %1").arg(reply->errorString()));
        reply->deleteLater();
        return;
    }

    QByteArray response = reply->readAll();
    reply->deleteLater();

    QJsonDocument doc = QJsonDocument::fromJson(response);
    if (!doc.isNull() && doc.isObject()) {
        QJsonObject obj = doc.object();
        if (obj.contains("choices") && obj["choices"].isArray()) {
            QJsonArray choices = obj["choices"].toArray();
            if (!choices.isEmpty() && choices.first().isObject()) {
                QJsonObject choice = choices.first().toObject();
                if (choice.contains("message") && choice["message"].isObject()) {
                    QString aiText = choice["message"].toObject()["content"].toString().trimmed();
                    
                    statusLabel->setText(mode == Description ? tr("Description received:") : tr("Extracted text:"));
                    
                    QString htmlContent = markdownToHtml(aiText);
                    QString currentHtml = resultBrowser->toHtml();
                    int bodyEndPos = currentHtml.lastIndexOf("</body>");
                    if (bodyEndPos != -1) {
                        currentHtml.insert(bodyEndPos, htmlContent);
                        resultBrowser->setHtml(currentHtml);
                    } else {
                        resultBrowser->setHtml(htmlContent);
                    }
                    
                    copyButton->setEnabled(true);
                    return;
                }
            }
        }
    }
    statusLabel->setText(tr("Error: Unexpected response format."));
}

void QVAIToolsDialog::onCopyClicked()
{
    // Copy plain text version
    QClipboard *clipboard = qvApp->clipboard();
    clipboard->setText(resultBrowser->toPlainText());
}

QString QVAIToolsDialog::markdownToHtml(const QString &markdown)
{
    QString html = markdown;
    html = processMath(html);
    html = html.toHtmlEscaped();
    
    html.replace(QRegularExpression("^### (.+)$", QRegularExpression::MultilineOption), "<h3>\\1</h3>");
    html.replace(QRegularExpression("^## (.+)$", QRegularExpression::MultilineOption), "<h2>\\1</h2>");
    html.replace(QRegularExpression("^# (.+)$", QRegularExpression::MultilineOption), "<h1>\\1</h1>");
    html.replace(QRegularExpression("```([\\s\\S]*?)```"), "<pre><code>\\1</code></pre>");
    html.replace(QRegularExpression("`([^`]+)`"), "<code>\\1</code>");
    html.replace(QRegularExpression("\\*\\*([^*]+)\\*\\*"), "<strong>\\1</strong>");
    html.replace(QRegularExpression("__([^_]+)__"), "<strong>\\1</strong>");
    html.replace(QRegularExpression("\\*([^*]+)\\*"), "<em>\\1</em>");
    html.replace(QRegularExpression("_([^_]+)_"), "<em>\\1</em>");
    html.replace(QRegularExpression("\\[([^\\]]+)\\]\\(([^)]+)\\)"), "<a href=\"\\2\">\\1</a>");
    html.replace(QRegularExpression("^&gt; (.+)$", QRegularExpression::MultilineOption), "<blockquote>\\1</blockquote>");
    html.replace(QRegularExpression("^- (.+)$", QRegularExpression::MultilineOption), "<li>\\1</li>");
    html.replace(QRegularExpression("^\\* (.+)$", QRegularExpression::MultilineOption), "<li>\\1</li>");
    html.replace(QRegularExpression("^\\d+\\. (.+)$", QRegularExpression::MultilineOption), "<li>\\1</li>");
    html.replace(QRegularExpression("(<li>.*</li>)+"), "<ul>\\0</ul>");
    html.replace("\n\n", "<br><br>");
    html.replace("\n", "<br>");
    
    return html;
}

QString QVAIToolsDialog::processMath(const QString &text)
{
    QString result = text;
    QRegularExpression displayMath(R"(\$\$([^\$]+)\$\$)");
    result.replace(displayMath, "<div style=\"text-align: center; font-style: italic; margin: 8px 0;\">\\1</div>");
    QRegularExpression inlineMath(R"(\$([^\$]+)\$)");
    result.replace(inlineMath, "<i>\\1</i>");
    return result;
}

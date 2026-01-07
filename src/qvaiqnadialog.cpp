#include "qvaiqnadialog.h"
#include "qvapplication.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QFile>
#include <QDebug>
#include <QScrollBar>
#include <QRegularExpression>

QVAIQnADialog::QVAIQnADialog(QWidget *parent) : QDialog(parent)
{
    setWindowTitle(tr("AI Q&A"));
    setMinimumSize(500, 600);
    
    auto *layout = new QVBoxLayout(this);
    
    chatHistory = new QTextBrowser(this);
    chatHistory->setOpenExternalLinks(false);
    chatHistory->setReadOnly(true);
    
    // Simple styling that matches Qt's native look
    chatHtml = R"(
<!DOCTYPE html>
<html>
<head>
<style>
body {
    font-family: system-ui, -apple-system, sans-serif;
    margin: 8px;
    line-height: 1.5;
}
.message {
    margin-bottom: 12px;
}
.role {
    font-weight: bold;
    margin-bottom: 2px;
}
.user-role {
    color: #0066cc;
}
.ai-role {
    color: #006600;
}
.system-role {
    color: #666666;
    font-style: italic;
}
.content {
    margin-left: 8px;
}
code {
    background: #f0f0f0;
    padding: 2px 4px;
    border-radius: 3px;
    font-family: monospace;
    font-size: 0.95em;
}
pre {
    background: #f5f5f5;
    border: 1px solid #ddd;
    padding: 10px;
    border-radius: 4px;
    overflow-x: auto;
    margin: 8px 0;
}
pre code {
    background: none;
    padding: 0;
}
blockquote {
    border-left: 3px solid #ccc;
    margin: 8px 0;
    padding-left: 10px;
    color: #555;
}
ul, ol {
    margin: 4px 0;
    padding-left: 20px;
}
strong {
    font-weight: bold;
}
em {
    font-style: italic;
}
h1, h2, h3 { margin: 10px 0 6px 0; }
h1 { font-size: 1.4em; }
h2 { font-size: 1.2em; }
h3 { font-size: 1.1em; }
</style>
</head>
<body>
</body>
</html>
)";
    
    chatHistory->setHtml(chatHtml);
    layout->addWidget(chatHistory);
    
    auto *inputLayout = new QHBoxLayout();
    inputField = new QLineEdit(this);
    inputField->setPlaceholderText(tr("Ask a question about the image..."));
    connect(inputField, &QLineEdit::returnPressed, this, &QVAIQnADialog::onSendClicked);
    
    sendButton = new QPushButton(tr("Send"), this);
    connect(sendButton, &QPushButton::clicked, this, &QVAIQnADialog::onSendClicked);
    
    inputLayout->addWidget(inputField);
    inputLayout->addWidget(sendButton);
    layout->addLayout(inputLayout);
    
    auto *bottomLayout = new QHBoxLayout();
    clearButton = new QPushButton(tr("Clear Chat"), this);
    connect(clearButton, &QPushButton::clicked, this, &QVAIQnADialog::onClearClicked);
    
    progressBar = new QProgressBar(this);
    progressBar->setVisible(false);
    progressBar->setMaximum(0);
    
    bottomLayout->addWidget(clearButton);
    bottomLayout->addStretch();
    bottomLayout->addWidget(progressBar);
    layout->addLayout(bottomLayout);
    
    networkManager = new QNetworkAccessManager(this);
    connect(networkManager, &QNetworkAccessManager::finished, this, &QVAIQnADialog::onNetworkReply);
}

void QVAIQnADialog::setImage(const QFileInfo &fileInfo)
{
    currentFileInfo = fileInfo;
    appendMessage("System", tr("Working with image: %1").arg(fileInfo.fileName()));
}

void QVAIQnADialog::onSendClicked()
{
    QString query = inputField->text().trimmed();
    if (query.isEmpty()) return;
    
    inputField->clear();
    appendMessage("You", query);
    sendQuery(query);
}

void QVAIQnADialog::onClearClicked()
{
    chatHistory->setHtml(chatHtml);
    if (currentFileInfo.exists()) {
        setImage(currentFileInfo);
    }
}

void QVAIQnADialog::appendMessage(const QString &role, const QString &text, bool isMarkdown)
{
    QString roleClass = "system-role";
    if (role == "You") {
        roleClass = "user-role";
    } else if (role == "AI") {
        roleClass = "ai-role";
    }
    
    QString content = text;
    if (isMarkdown && role == "AI") {
        content = markdownToHtml(text);
    } else {
        content = content.toHtmlEscaped().replace("\n", "<br>");
    }
    
    QString messageHtml = QString(R"(
<div class="message">
    <div class="role %1">%2:</div>
    <div class="content">%3</div>
</div>
)").arg(roleClass, role, content);
    
    // Insert before closing body tag
    QString currentHtml = chatHistory->toHtml();
    int bodyEndPos = currentHtml.lastIndexOf("</body>");
    if (bodyEndPos != -1) {
        currentHtml.insert(bodyEndPos, messageHtml);
        chatHistory->setHtml(currentHtml);
    }
    
    // Scroll to bottom
    QScrollBar *scrollBar = chatHistory->verticalScrollBar();
    scrollBar->setValue(scrollBar->maximum());
}

QString QVAIQnADialog::markdownToHtml(const QString &markdown)
{
    QString html = markdown;
    
    // Process math first
    html = processMath(html);
    
    // Escape HTML
    html = html.toHtmlEscaped();
    
    // Headers
    html.replace(QRegularExpression("^### (.+)$", QRegularExpression::MultilineOption), "<h3>\\1</h3>");
    html.replace(QRegularExpression("^## (.+)$", QRegularExpression::MultilineOption), "<h2>\\1</h2>");
    html.replace(QRegularExpression("^# (.+)$", QRegularExpression::MultilineOption), "<h1>\\1</h1>");
    
    // Code blocks
    html.replace(QRegularExpression("```([\\s\\S]*?)```"), "<pre><code>\\1</code></pre>");
    
    // Inline code
    html.replace(QRegularExpression("`([^`]+)`"), "<code>\\1</code>");
    
    // Bold
    html.replace(QRegularExpression("\\*\\*([^*]+)\\*\\*"), "<strong>\\1</strong>");
    html.replace(QRegularExpression("__([^_]+)__"), "<strong>\\1</strong>");
    
    // Italic
    html.replace(QRegularExpression("\\*([^*]+)\\*"), "<em>\\1</em>");
    html.replace(QRegularExpression("_([^_]+)_"), "<em>\\1</em>");
    
    // Links
    html.replace(QRegularExpression("\\[([^\\]]+)\\]\\(([^)]+)\\)"), "<a href=\"\\2\">\\1</a>");
    
    // Blockquotes
    html.replace(QRegularExpression("^&gt; (.+)$", QRegularExpression::MultilineOption), "<blockquote>\\1</blockquote>");
    
    // Lists
    html.replace(QRegularExpression("^- (.+)$", QRegularExpression::MultilineOption), "<li>\\1</li>");
    html.replace(QRegularExpression("^\\* (.+)$", QRegularExpression::MultilineOption), "<li>\\1</li>");
    html.replace(QRegularExpression("^\\d+\\. (.+)$", QRegularExpression::MultilineOption), "<li>\\1</li>");
    
    // Wrap consecutive <li> in ul
    html.replace(QRegularExpression("(<li>.*</li>)+"), "<ul>\\0</ul>");
    
    // Line breaks
    html.replace("\n\n", "<br><br>");
    html.replace("\n", "<br>");
    
    return html;
}

QString QVAIQnADialog::processMath(const QString &text)
{
    QString result = text;
    
    // Display math: $$...$$
    QRegularExpression displayMath(R"(\$\$([^\$]+)\$\$)");
    result.replace(displayMath, "<div style=\"text-align: center; font-style: italic; margin: 8px 0;\">\\1</div>");
    
    // Inline math: $...$
    QRegularExpression inlineMath(R"(\$([^\$]+)\$)");
    result.replace(inlineMath, "<i>\\1</i>");
    
    return result;
}

void QVAIQnADialog::sendQuery(const QString &query)
{
    progressBar->setVisible(true);
    sendButton->setEnabled(false);
    
    QString apiKey = qEnvironmentVariable("QVIEW_AI_API_KEY", "");
    if (apiKey.isEmpty()) {
        apiKey = qvApp->getSettingsManager().getString("airename/apikey");
    }
    
    if (apiKey.isEmpty()) {
        appendMessage("System", tr("Error: API key not configured."));
        progressBar->setVisible(false);
        sendButton->setEnabled(true);
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
        
    // Read image
    QFile imageFile(currentFileInfo.absoluteFilePath());
    if (!imageFile.open(QIODevice::ReadOnly)) {
        appendMessage("System", tr("Error: Could not read image file."));
        progressBar->setVisible(false);
        sendButton->setEnabled(true);
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
    textContent["text"] = query;
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
    
    networkManager->post(request, QJsonDocument(payload).toJson());
}

void QVAIQnADialog::onNetworkReply(QNetworkReply *reply)
{
    progressBar->setVisible(false);
    sendButton->setEnabled(true);
    
    if (reply->error() != QNetworkReply::NoError) {
        appendMessage("System", tr("Error: %1").arg(reply->errorString()));
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
                    appendMessage("AI", aiText, true);
                    return;
                }
            }
        }
    }
    appendMessage("System", tr("Error: Unexpected response format."));
}

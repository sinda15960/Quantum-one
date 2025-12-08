#include "simpleqrcode.h"
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QUrl>
#include <QPixmap>
#include <QDebug>
#include <QLabel>

// Simple QR Code generator that works with Qt 5.9.9
// Uses online API as fallback - simple and reliable

QImage SimpleQRCode::generateQRCode(const QString &text, int size)
{
    // Method 1: Use online QR code API (simple and reliable)
    // This works perfectly with Qt 5.9.9 and has no encoding issues
    QString encodedText = QUrl::toPercentEncoding(text);
    QString url = QString("https://api.qrserver.com/v1/create-qr-code/?size=%1x%1&data=%2")
                     .arg(size)
                     .arg(encodedText);
    
    QNetworkAccessManager manager;
    QNetworkRequest request = QNetworkRequest(QUrl(url));
    QNetworkReply *reply = manager.get(request);
    
    QEventLoop loop;
    QObject::connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    loop.exec();
    
    if (reply->error() == QNetworkReply::NoError) {
        QByteArray imageData = reply->readAll();
        QImage qrImage;
        if (qrImage.loadFromData(imageData)) {
            return qrImage.scaled(size, size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        }
    }
    
    // Fallback: Create a simple placeholder if online generation fails
    QImage placeholder(size, size, QImage::Format_RGB32);
    placeholder.fill(Qt::white);
    QPainter painter(&placeholder);
    painter.setPen(Qt::black);
    painter.drawText(QRect(0, 0, size, size), Qt::AlignCenter, 
                     QString("QR Code\n%1").arg(text.left(20)));
    return placeholder;
}

QImage SimpleQRCode::generateQRCodeOnline(const QString &text, int size)
{
    return generateQRCode(text, size);
}


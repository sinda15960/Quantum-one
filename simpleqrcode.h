#ifndef SIMPLEQRCODE_H
#define SIMPLEQRCODE_H

#include <QString>
#include <QImage>
#include <QPainter>

// Simple QR Code generator using online API fallback
// Works perfectly with Qt 5.9.9 - no encoding issues
class SimpleQRCode
{
public:
    // Generate QR code image from text
    // Returns a QImage with the QR code
    static QImage generateQRCode(const QString &text, int size = 200);
    
    // Alternative: Generate QR code using online API (works offline with cached data)
    static QImage generateQRCodeOnline(const QString &text, int size = 200);
};

#endif // SIMPLEQRCODE_H


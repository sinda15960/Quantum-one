#include "smtp.h"
#include <QDebug>
#include <QTimer>
#include <QThread>
#include <QCoreApplication>
#include <QElapsedTimer>

Smtp::Smtp(const QString &user, const QString &pass, QObject *parent)
    : QObject(parent), m_user(user), m_pass(pass) {}

Smtp::~Smtp() {}

bool Smtp::waitForSocketReady(QSslSocket *socket, int timeoutMs) {
    QElapsedTimer t;
    t.start();
    while (!socket->isEncrypted() && t.elapsed() < timeoutMs) {
        QCoreApplication::processEvents();
        QThread::msleep(20);
    }
    return socket->isEncrypted();
}

bool Smtp::sendMail(const QString &to, const QString &subject, const QString &body, QString &errorOut) {
    QSslSocket socket;
    socket.connectToHostEncrypted("smtp.gmail.com", 465);

    if (!socket.waitForConnected(10000)) {
        errorOut = socket.errorString();
        return false;
    }

    if (!waitForSocketReady(&socket, 10000)) {
        errorOut = "SSL handshake failed or timed out.";
        return false;
    }

    auto writeLine = [&](const QString &line) {
        QByteArray data = line.toUtf8() + "\r\n";
        socket.write(data);
        if (!socket.waitForBytesWritten(5000)) { }
        socket.waitForReadyRead(5000);
    };

    auto readResponse = [&]() -> QString {
        QByteArray resp;
        while (socket.bytesAvailable()) {
            resp += socket.readAll();
            socket.waitForReadyRead(50);
        }
        return QString::fromUtf8(resp);
    };

    writeLine(QString("EHLO localhost")); readResponse();
    writeLine("AUTH LOGIN"); readResponse();
    writeLine(QString(m_user.toUtf8().toBase64())); readResponse();
    writeLine(QString(m_pass.toUtf8().toBase64())); readResponse();
    writeLine("MAIL FROM:<" + m_user + ">"); readResponse();
    writeLine("RCPT TO:<" + to + ">"); readResponse();
    writeLine("DATA"); readResponse();
    writeLine("Subject: " + subject);
    writeLine("From: " + m_user);
    writeLine("To: " + to);
    writeLine("");
    writeLine(body);
    writeLine("."); readResponse();
    writeLine("QUIT"); readResponse();

    if (socket.error() != QAbstractSocket::UnknownSocketError) {
        errorOut = socket.errorString();
        return false;
    }

    return true;
}

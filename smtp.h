#ifndef SMTP_H
#define SMTP_H

#include <QObject>
#include <QSslSocket>

class Smtp : public QObject {
    Q_OBJECT

public:
    explicit Smtp(const QString &user, const QString &pass, QObject *parent = nullptr);
    ~Smtp();

    bool sendMail(const QString &to, const QString &subject, const QString &body, QString &errorOut);

private:
    QString m_user;
    QString m_pass;
    bool waitForSocketReady(QSslSocket *socket, int timeoutMs = 10000);
};

#endif // SMTP_H

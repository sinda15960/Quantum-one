#ifndef VOICETOTEXT_H
#define VOICETOTEXT_H

#include <QObject>
#include <QAudioFormat>
#include <QAudioSource>
#include <QString>
#include <QWebSocket>
#include <QAbstractSocket>
#include <memory>

class QIODevice;
class QJsonValue;

class VoiceToText : public QObject
{
    Q_OBJECT

public:
    explicit VoiceToText(QObject *parent = nullptr);
    ~VoiceToText();

    void setApiKey(const QString &key);
    [[nodiscard]] QString apiKey() const;
    [[nodiscard]] bool hasApiKey() const;

    void setVerboseTranscription(bool enabled);
    void setSampleRate(int sampleRate);

    bool start();
    void stop();
    [[nodiscard]] bool isRecording() const;

signals:
    void statusChanged(const QString &status);
    void errorOccurred(const QString &errorMessage);
    void interimMessageReceived(const QString &text);
    void finalMessageReceived(const QString &role, const QString &text);

private slots:
    void handleSocketConnected();
    void handleSocketClosed();
    void handleSocketError(QAbstractSocket::SocketError error);
    void handleSocketMessage(const QString &message);
    void handleAudioReady();

private:
    void configureAudioFormat();
    void resetAudioPipeline();
    void sendSessionSettings();
    void sendAudioChunk(const QByteArray &pcmData);
    [[nodiscard]] QString extractContent(const QJsonValue &value) const;

    QString m_apiKey;
    bool m_verboseTranscription{true};
    int m_sampleRate{16000};
    bool m_isRecording{false};

    QWebSocket m_socket;
    QAudioFormat m_format;
    QAudioSource *m_audioSource{nullptr};
    QIODevice *m_audioDevice{nullptr};
};

#endif // VOICETOTEXT_H

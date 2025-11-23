#include "voicetotext.h"

#include <QAudioSource>
#include <QIODevice>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QMediaDevices>
#include <QStringList>
#include <QUrl>
#include <QUrlQuery>

// Local key include (optional)
// If you'd like to embed the key directly in `voicetotext.cpp` for local tests,
// create `local_hume_key.h` from `local_hume_key.h.example` and add
// `DEFINES += HAVE_LOCAL_HUME_KEY` to your local `.pro.user` file or add
// `-DHAVE_LOCAL_HUME_KEY` to the compiler flags for your local build.
// Do NOT commit `local_hume_key.h` or the `.pro.user` change with your key.
#if defined(HAVE_LOCAL_HUME_KEY)
#include "local_hume_key.h"
#endif

namespace {
constexpr auto kHumeEndpoint = "wss://api.hume.ai/v0/evi/chat";
}

VoiceToText::VoiceToText(QObject *parent)
    : QObject(parent)
{
    configureAudioFormat();

    connect(&m_socket, &QWebSocket::connected, this, &VoiceToText::handleSocketConnected);
    connect(&m_socket, &QWebSocket::disconnected, this, &VoiceToText::handleSocketClosed);
    connect(&m_socket, &QWebSocket::textMessageReceived, this, &VoiceToText::handleSocketMessage);
    connect(&m_socket,
            QOverload<QAbstractSocket::SocketError>::of(&QWebSocket::errorOccurred),
            this,
            &VoiceToText::handleSocketError);

    // If a compile-time API key is defined with qmake (DEFINES += HUME_API_KEY=\"<key>\"),
    // use it as a default so the app will connect without environment variables.
    // IMPORTANT: This is a convenience for local testing only — do NOT commit the
    // key to source control. Prefer environment variables or the .hume_api_key file.
    // If a compile-time macro is present we attempt to convert it to a string.
    // This avoids syntax problems if the macros are added with or without quotes.


    // Alternatively, you can hard-code a key directly here for local-only testing.
    // Uncomment and set your key only in your local working copy; do not commit it.
    // Example:
    setApiKey(QStringLiteral("nCkDcVxCcO78RGwrW2UvZxHJaA0PAxAh7t0tv0kW6TikIudO"));

}

VoiceToText::~VoiceToText()
{
    stop();
}

void VoiceToText::configureAudioFormat()
{
    m_format = QAudioFormat();
    m_format.setChannelCount(1);
    m_format.setSampleRate(m_sampleRate);
    m_format.setSampleFormat(QAudioFormat::Int16);
}

void VoiceToText::setApiKey(const QString &key)
{
    m_apiKey = key.trimmed();
}

QString VoiceToText::apiKey() const
{
    return m_apiKey;
}

bool VoiceToText::hasApiKey() const
{
    return !m_apiKey.isEmpty();
}

void VoiceToText::setVerboseTranscription(bool enabled)
{
    m_verboseTranscription = enabled;
}

void VoiceToText::setSampleRate(int sampleRate)
{
    if (sampleRate <= 0 || m_sampleRate == sampleRate) {
        return;
    }
    m_sampleRate = sampleRate;
    configureAudioFormat();
}

bool VoiceToText::start()
{
    if (!hasApiKey()) {
        emit errorOccurred(tr("Missing Hume API key. Define the HUME_API_KEY environment variable."));
        return false;
    }

    if (m_socket.state() == QAbstractSocket::ConnectingState ||
        m_socket.state() == QAbstractSocket::ConnectedState) {
        return true;
    }

    QUrl url(QString::fromLatin1(kHumeEndpoint));
    QUrlQuery query;
    query.addQueryItem(QStringLiteral("api_key"), m_apiKey);
    if (m_verboseTranscription) {
        query.addQueryItem(QStringLiteral("verbose_transcription"), QStringLiteral("true"));
    }
    url.setQuery(query);

    m_socket.open(url);
    emit statusChanged(tr("Connecting to Hume…"));
    return true;
}

void VoiceToText::stop()
{
    if (m_socket.state() != QAbstractSocket::UnconnectedState) {
        m_socket.close();
    }
    resetAudioPipeline();

    if (m_isRecording) {
        m_isRecording = false;
        emit statusChanged(tr("Recording stopped."));
    }
}

bool VoiceToText::isRecording() const
{
    return m_isRecording;
}

void VoiceToText::handleSocketConnected()
{
    m_isRecording = true;
    emit statusChanged(tr("Streaming audio to Hume…"));

    sendSessionSettings();

    const QAudioDevice device = QMediaDevices::defaultAudioInput();
    if (!device.isNull()) {
        QAudioFormat format = m_format;
        if (!device.isFormatSupported(format)) {
            format = device.preferredFormat();
            format.setChannelCount(1);
            format.setSampleRate(m_sampleRate);
            format.setSampleFormat(QAudioFormat::Int16);
        }
        // Allocate a QObject-based audio source with this as parent so Qt's
        // object tree manages cleanup. Don't use unique_ptr to avoid double-delete.
        m_audioSource = new QAudioSource(device, format, this);
    } else {
        emit errorOccurred(tr("No microphone detected."));
        stop();
        return;
    }

    m_audioDevice = m_audioSource->start();
    if (!m_audioDevice) {
        emit errorOccurred(tr("Unable to access the microphone stream."));
        stop();
        return;
    }

    connect(m_audioDevice, &QIODevice::readyRead, this, &VoiceToText::handleAudioReady);
}

void VoiceToText::handleSocketClosed()
{
    if (m_isRecording) {
        m_isRecording = false;
        emit statusChanged(tr("Disconnected from Hume."));
    }
    resetAudioPipeline();
}

void VoiceToText::handleSocketError(QAbstractSocket::SocketError)
{
    emit errorOccurred(m_socket.errorString());
    stop();
}

void VoiceToText::handleSocketMessage(const QString &message)
{
    const QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (!doc.isObject()) {
        return;
    }

    const QJsonObject obj = doc.object();
    const QString type = obj.value(QStringLiteral("type")).toString();

    if (type == QStringLiteral("user_message") || type == QStringLiteral("assistant_message")) {
        const QString content = extractContent(obj.value(QStringLiteral("message")).toObject().value(QStringLiteral("content")));
        if (content.isEmpty()) {
            return;
        }
        const bool interim = obj.value(QStringLiteral("interim")).toBool(false);
        if (interim) {
            emit interimMessageReceived(content);
        } else {
            const QString role = type == QStringLiteral("user_message") ? QStringLiteral("user") : QStringLiteral("assistant");
            emit finalMessageReceived(role, content);
        }
    } else if (type == QStringLiteral("websocket_error")) {
        emit errorOccurred(obj.value(QStringLiteral("message")).toString());
    } else if (type == QStringLiteral("chat_end_message")) {
        emit statusChanged(tr("Chat session ended."));
        stop();
    }
}

void VoiceToText::handleAudioReady()
{
    if (!m_audioDevice || m_socket.state() != QAbstractSocket::ConnectedState) {
        return;
    }

    const QByteArray pcmData = m_audioDevice->readAll();
    if (pcmData.isEmpty()) {
        return;
    }

    sendAudioChunk(pcmData);
}

void VoiceToText::resetAudioPipeline()
{
    if (m_audioDevice) {
        disconnect(m_audioDevice, &QIODevice::readyRead, this, &VoiceToText::handleAudioReady);
        m_audioDevice = nullptr;
    }
    if (m_audioSource) {
        m_audioSource->stop();
        // Qt will delete the object due to parent ownership on destructor, but
        // we null out our pointer here to be safe.
        m_audioSource = nullptr;
    }
}

void VoiceToText::sendSessionSettings()
{
    QJsonObject audio;
    audio.insert(QStringLiteral("encoding"), QStringLiteral("linear16"));
    audio.insert(QStringLiteral("sample_rate"), m_sampleRate);
    audio.insert(QStringLiteral("channels"), m_format.channelCount());

    QJsonObject settings;
    settings.insert(QStringLiteral("type"), QStringLiteral("session_settings"));
    settings.insert(QStringLiteral("audio"), audio);

    m_socket.sendTextMessage(QJsonDocument(settings).toJson(QJsonDocument::Compact));
}

void VoiceToText::sendAudioChunk(const QByteArray &pcmData)
{
    if (pcmData.isEmpty()) {
        return;
    }

    QJsonObject payload;
    payload.insert(QStringLiteral("type"), QStringLiteral("audio_input"));
    payload.insert(QStringLiteral("data"), QString::fromLatin1(pcmData.toBase64()));

    m_socket.sendTextMessage(QJsonDocument(payload).toJson(QJsonDocument::Compact));
}

QString VoiceToText::extractContent(const QJsonValue &value) const
{
    if (value.isString()) {
        return value.toString();
    }

    if (value.isArray()) {
        QStringList parts;
        const QJsonArray array = value.toArray();
        parts.reserve(array.size());
        for (const QJsonValue &item : array) {
            if (item.isString()) {
                parts << item.toString();
            } else if (item.isObject()) {
                const QJsonObject obj = item.toObject();
                const QString text = obj.value(QStringLiteral("text")).toString();
                if (!text.isEmpty()) {
                    parts << text;
                }
            }
        }
        return parts.join(QLatin1Char(' '));
    }

    if (value.isObject()) {
        return value.toObject().value(QStringLiteral("text")).toString();
    }

    return {};
}

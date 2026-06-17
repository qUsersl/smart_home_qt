#include "audiocapture.h"

#include <QAudioDeviceInfo>
#include <QDebug>
#include <QMessageBox>

namespace {

void normalizePcm16File(const QString &filename)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadWrite)) {
        return;
    }

    QByteArray data = file.readAll();
    if (data.size() < static_cast<int>(sizeof(qint16))) {
        return;
    }

    qint16 *samples = reinterpret_cast<qint16 *>(data.data());
    const int sampleCount = data.size() / static_cast<int>(sizeof(qint16));

    int peak = 0;
    for (int i = 0; i < sampleCount; ++i) {
        const int value = qAbs(static_cast<int>(samples[i]));
        if (value > peak) {
            peak = value;
        }
    }

    if (peak == 0) {
        return;
    }

    const double targetPeak = 28000.0;
    const double gain = qBound(1.0, targetPeak / static_cast<double>(peak), 6.0);
    if (gain <= 1.05) {
        return;
    }

    for (int i = 0; i < sampleCount; ++i) {
        const int amplified = qRound(static_cast<double>(samples[i]) * gain);
        samples[i] = static_cast<qint16>(qBound(-32768, amplified, 32767));
    }

    file.seek(0);
    file.write(data);
    file.resize(data.size());
    file.close();
}

}

AudioCapture::AudioCapture(QObject *parent) : QObject(parent)
{
    pAudioInput = nullptr;
    pFile = nullptr;
}

void AudioCapture::startCapture(QString filename)
{
    QAudioDeviceInfo audioDeviceInfo = QAudioDeviceInfo::defaultInputDevice();
    if (audioDeviceInfo.isNull()) {
        QMessageBox::information(nullptr, tr("Record"), tr("当前没有可用的录音设备"));
        return;
    }

    pFile = new QFile;
    pFile->setFileName(filename);
    if (!pFile->open(QIODevice::WriteOnly | QIODevice::Truncate)) {
        QMessageBox::information(nullptr, tr("Record"),
                                 tr("无法打开文件: %1").arg(pFile->errorString()));
        delete pFile;
        pFile = nullptr;
        return;
    }

    QAudioFormat format;
    format.setSampleRate(16000);
    format.setChannelCount(1);
    format.setSampleSize(16);
    format.setCodec("audio/pcm");
    format.setByteOrder(QAudioFormat::LittleEndian);
    format.setSampleType(QAudioFormat::SignedInt);

    if (!audioDeviceInfo.isFormatSupported(format)) {
        format = audioDeviceInfo.nearestFormat(format);
    }

    if (pAudioInput != nullptr) {
        delete pAudioInput;
        pAudioInput = nullptr;
    }

    m_format = format;
    pAudioInput = new QAudioInput(format, this);
    pAudioInput->setVolume(1.0);
    pAudioInput->start(pFile);
}

void AudioCapture::stopCapture()
{
    QString filename;

    if (pAudioInput != nullptr) {
        pAudioInput->stop();
        delete pAudioInput;
        pAudioInput = nullptr;
    }

    if (pFile != nullptr) {
        pFile->flush();
        filename = pFile->fileName();
        pFile->close();
        delete pFile;
        pFile = nullptr;
    }

    if (!filename.isEmpty()) {
        normalizePcm16File(filename);
    }
}

QAudioFormat AudioCapture::captureFormat() const
{
    return m_format;
}

AudioCapture::~AudioCapture()
{
    if (pAudioInput != nullptr) {
        delete pAudioInput;
        pAudioInput = nullptr;
    }

    if (pFile != nullptr) {
        delete pFile;
        pFile = nullptr;
    }
}

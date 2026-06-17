#ifndef AUDIOCAPTURE_H
#define AUDIOCAPTURE_H

#include <QObject>
#include <QAudioFormat>
#include <QAudioInput>
#include <QFile>

class AudioCapture : public QObject
{
    Q_OBJECT
public:
    explicit AudioCapture(QObject *parent = nullptr);
    void startCapture(QString filename);
    void stopCapture();
    QAudioFormat captureFormat() const;
    ~AudioCapture();

private:
    QAudioInput *pAudioInput;
    QFile       *pFile;
    QAudioFormat m_format;
};

#endif // AUDIOCAPTURE_H

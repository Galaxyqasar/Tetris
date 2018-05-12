#ifndef SOUNDSYSTEM_H
#define SOUNDSYSTEM_H

#include <QDir>
#include <QSound>
#include <QObject>
#include <QMediaPlayer>
#include <QMediaPlaylist>

enum SoundType{
    rowCompleted,
    rotated,
    blockLanded,
    buttonClicked
};

class SoundSystem : public QObject
{
    Q_OBJECT
public:
    explicit SoundSystem(QObject *parent = nullptr);

signals:

public slots:
    void startBackground();
    void stopBackground();
    void playSound(SoundType type);

private:
    QMediaPlayer *backGroundMusikPlayer;
    QMediaPlaylist *backGroundMusik;
};

#endif // SOUNDSYSTEM_H

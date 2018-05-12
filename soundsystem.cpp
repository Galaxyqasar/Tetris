#include "soundsystem.h"

SoundSystem::SoundSystem(QObject *parent) : QObject(parent)
{
    backGroundMusikPlayer = new QMediaPlayer(this);
    backGroundMusik = new QMediaPlaylist(this);
    QDir soundDir = QString(QDir::currentPath()+"/Sounds/");
    if(soundDir.exists()){
        QList <QString> Sounds;
        QString soundPath = QDir::currentPath()+"/Sounds/";
        for(int i = 0; i < soundDir.entryList().size(); i++){
            Sounds.append(soundPath+soundDir.entryList().at(i));
        }
        Sounds.removeFirst(); Sounds.removeFirst();
        for(int i = 0; i < Sounds.size(); i++){
            if(Sounds.at(i).contains("background"))
                backGroundMusik->addMedia(QMediaContent(QUrl::fromLocalFile(Sounds.at(i))));
        }
        backGroundMusik->setPlaybackMode(QMediaPlaylist::Random);
        backGroundMusik->shuffle();
        backGroundMusikPlayer->setPlaylist(backGroundMusik);
    }
}

void SoundSystem::startBackground()
{
    backGroundMusikPlayer->play();
}

void SoundSystem::stopBackground()
{
    backGroundMusikPlayer->stop();
    backGroundMusik->shuffle();
}

void SoundSystem::playSound(SoundType type)
{
    switch(type){
    case rowCompleted:{
            QSound::play("");
        }
        break;
    case rotated:{
            QSound::play("");
        }
        break;
    case blockLanded:{
            QSound::play("");
        }
        break;
    case buttonClicked:{
            QSound::play(QDir::currentPath()+"/Sounds/button.wav");
        }
        break;
    default:
        break;
    }
}

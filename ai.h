#ifndef AI_H
#define AI_H

#include <QObject>
#include <QFile>
#include "net.h"

class AI : public QObject
{
    Q_OBJECT
public:
    explicit AI(QObject *parent = nullptr);
signals:

public slots:
    void createNet(QList<unsigned> topology);
    void loadNet(QString filename);
    void saveNet(QString filename);
    QList<double> nextPass(QList<double> input, QList<double> target);

private:
    Net net;
};

#endif // AI_H

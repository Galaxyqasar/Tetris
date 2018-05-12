#include "ai.h"
#include <QVector>
#include <QDebug>

AI::AI(QObject *parent) : QObject(parent){
    ;
}

void AI::createNet(QList<unsigned> topology)
{
    net = Net(topology.toVector().toStdVector());
}

void AI::loadNet(QString filename)
{
    ;
}

void AI::saveNet(QString filename)
{
    ;
}

QList<double> AI::nextPass(QList<double> input, QList<double> target = QList<double>())
{
    if(target.isEmpty()){
        net.feedForward(input.toVector().toStdVector());
        vector<double> result;
        net.getResults(result);
        qDebug()<<result;
        return QList<double>::fromVector(QVector<double>::fromStdVector(result));
    }
    else{
        net.feedForward(input.toVector().toStdVector());
        vector<double> result;
        net.getResults(result);
        net.backProp(target.toVector().toStdVector());
        qDebug()<<result;
        return QList<double>::fromVector(QVector<double>::fromStdVector(result));
    }
}

#include "tetris.h"
#include <time.h>
#include <QApplication>

int main(int argc, char *argv[])
{
    srand((unsigned)time(NULL));

    QApplication a(argc, argv);
    Tetris w;
    w.show();

    QList<QString> arguments;
    for(int i = 0; i < argc; i++) arguments.append(QString(argv[i]));
    qDebug()<<arguments;
//    if(!arguments.contains("-nostd"))
//        w.loadMods(":/Blocks.xml");
    if(arguments.contains("-mod"))
        w.loadMods(arguments.at(arguments.indexOf("-mod")+1));


    return a.exec();
}

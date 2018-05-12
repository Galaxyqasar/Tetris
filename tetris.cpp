#include "tetris.h"
#include "ui_tetris.h"

Tetris::Tetris(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Tetris)
{
    ui->setupUi(this);
    rowsCompleted = 0;
    inGame = false;
    running = false;
    AIenabled = false;
    ui->rowsLabel->setText(QString("Zeilen:\n"+QString::number(rowsCompleted)));
    mainGame = QImage(FIELD_WIDTH*TILE_RES, FIELD_HEIGHT*TILE_RES, QImage::Format_RGB888);
    nextImg = QImage(BLOCK_SIZE*TILE_RES, BLOCK_SIZE*TILE_RES, QImage::Format_RGB888);
    ui->mainGameDisplay->setAlignment(Qt::AlignCenter);
    ui->nextDisplay->setAlignment(Qt::AlignCenter);
    ui->mainGameDisplay->setMinimumSize(FIELD_WIDTH, FIELD_HEIGHT);
    ui->nextDisplay->setMinimumSize(BLOCK_SIZE,BLOCK_SIZE);
    soundSystem = new SoundSystem(this);
    moveTimer = new QTimer;
    moveTimer->setInterval(speed);
    connect(moveTimer, &QTimer::timeout, this, [&](){
        if(checkColissionBottom(scene.current))
            scene.current.y += 1;
        else
            nextBlock();
        renderScene();
#ifdef AI_ENABLED
        if(AIenabled){
            double move = ai.nextPass(getField(), QList<double>()).first();
            qDebug()<<move;
            action(move);
        }
        else
            ai.nextPass(getField(), QList<double>({0.9}));
#endif
    });
    loadMods(":/Blocks.xml");
    level = 0;
    nextLevel();
#ifdef AI_ENABLED
    ai.createNet(QList<unsigned>({280,280,280,140,70,35,1}));
#endif
}

Tetris::~Tetris()
{
    delete ui;
}

#ifdef AI_ENABLED

QList<double> Tetris::getField()
{
    double field[FIELD_HEIGHT*FIELD_WIDTH];
    for(int i = 0; i < FIELD_HEIGHT*FIELD_WIDTH; i++)
        field[i] = 0.0;
    for(int i = 0; i < scene.landed.size(); i++){
        Tile tmp = scene.landed.at(i);
        int index = tmp.x+tmp.y*FIELD_WIDTH;
        field[index] = 1.0;
    }
    for(int i = 0; i < scene.current.shape.size(); i++){
        QPoint tmp = scene.current.shape.at(i);
        tmp += QPoint(scene.current.x, scene.current.y);
        int index = tmp.x()+tmp.y()*FIELD_WIDTH;
        field[index] = 1.0;
    }
    QList<double> tmplist;
    for(int i = 0; i < FIELD_HEIGHT*FIELD_WIDTH; i++)
        tmplist.append(field[i]);
    qDebug()<<tmplist;
    return tmplist;
}

#endif

void Tetris::on_startButton_clicked()
{
    if(!inGame){
        setupBlocks();
        moveTimer->start();
        ui->startButton->setText("Stop");
        inGame = true;
        running = true;
        soundSystem->startBackground();
        score = 0;
    }
    else{
        scene.landed.clear();
        rowsCompleted = 0;
        moveTimer->stop();
        ui->startButton->setText("Start");
        inGame = false;
        soundSystem->stopBackground();
    }
    soundSystem->playSound(buttonClicked);
}

void Tetris::on_pauseButton_clicked()
{
    if(running){
        //pause
        ui->pauseButton->setText("Resume");
        moveTimer->stop();
        running = false;
    }
    else{
        //resume
        ui->pauseButton->setText("Pause");
        moveTimer->start();
        running = true;
    }
    soundSystem->playSound(buttonClicked);
}



void Tetris::on_saveButton_clicked()
{
#ifdef AI_ENABLED
    //save ai
#endif
    soundSystem->playSound(buttonClicked);
}

void Tetris::on_loadButton_clicked()
{
#ifdef AI_ENABLED
    //load ai
#endif
    soundSystem->playSound(buttonClicked);
}


void Tetris::setupBlocks()
{
    scene.next = blocks.at(rand()%(blocks.size()));
    scene.current = blocks.at(rand()%(blocks.size()));
    int rot = rand()%4;
    for(int i = 0; i < rot; i++)
        scene.next = rotateBlock(scene.next);
    rot = rand()%4;
    for(int i = 0; i < rot; i++)
        scene.current = rotateBlock(scene.current);
    renderNext();
    renderScene();
}

void Tetris::renderScene()
{
    QTime currentTime = QTime::currentTime();
    QList<Tile> tiles = scene.landed;
    Block current = scene.current;
    mainGame.fill(QColor(Qt::darkGray).darker(200));
    QPainter painter;
    painter.begin(&mainGame);
//    for(int x = 0; x < FIELD_WIDTH; x++)
//        painter.drawLine(x*TILE_RES, 0, x*TILE_RES, FIELD_HEIGHT*TILE_RES);
//    for(int y = 0; y < FIELD_HEIGHT; y++)
//        painter.drawLine(0, y*TILE_RES, FIELD_WIDTH*TILE_RES, y*TILE_RES);

    //current
    for(int i = 0; i < current.shape.size(); i++){
        painter.drawImage((current.shape.at(i).x()+current.x)*TILE_RES,\
                          (current.shape.at(i).y()+current.y)*TILE_RES,\
                          current.texture);
    }
    //tiles
    for(int i = 0; i < tiles.size(); i++){
        Tile tmp = tiles.at(i);
        painter.drawImage(tmp.x*TILE_RES, tmp.y*TILE_RES, tmp.texture);
    }
    ui->mainGameDisplay->setPixmap(QPixmap::fromImage(mainGame.scaled(\
                ui->mainGameDisplay->width()-4, ui->mainGameDisplay->height()-4,\
                Qt::KeepAspectRatio)));
    ui->renderTimeLabel->setText("Time: "+QString::number(\
                QTime::currentTime().msec()-currentTime.msec())+" ms");
}

void Tetris::renderNext()
{
    Block next = scene.next;
    nextImg.fill(QColor(Qt::darkGray).darker(200));
    QPainter painter;
    painter.begin(&nextImg);
    for(int i = 0; i < next.shape.size(); i++){
        painter.drawImage(next.shape.at(i).x()*TILE_RES, next.shape.at(i).y()*TILE_RES, next.texture);
    }
    ui->nextDisplay->setPixmap(QPixmap::fromImage(nextImg.scaled(\
                ui->nextDisplay->width()-4, ui->nextDisplay->height()-4,\
                Qt::KeepAspectRatio)));
}

void Tetris::blockToTiles()
{
    Block current = scene.current;
    for(int i = 0; i < current.shape.size(); i++){
        Tile tile;
        tile.x = current.x + current.shape.at(i).x();
        tile.y = current.y + current.shape.at(i).y();
        tile.texture = current.texture;
        scene.landed.append(tile);
    }
}

Block Tetris::rotateBlock(Block block)
{
    Block rotated = block;
    for(int i = 0; i < block.shape.size(); i++){
        QPoint old = block.shape.at(i), tmp;
        old.setX(old.x()-2);
        old.setY(old.y()-2);
        tmp.setX(old.y());
        tmp.setY(old.x()*-1);
        tmp.setX(tmp.x()+2);
        tmp.setY(tmp.y()+2);
        rotated.shape.replace(i, tmp);
    }
    for(int i = 0; i < rotated.shape.size(); i++){
        QPoint current = rotated.shape.at(i);
        current +=QPoint(block.x, block.y);
        if(current.x() < 0 || current.x() > 13 || current.y() > 19)
            return block;
        for(int k = 0; k < scene.landed.size(); k++){
            if(current.x() == scene.landed.at(k).x && current.y() == scene.landed.at(k).y)
                return block;
        }
    }
    return centerBlock(rotated);
    return rotated;
}

Block Tetris::centerBlock(Block block)
{
    Block centered = block;
    for(int i = 0; i < centered.shape.size(); i++){
        centered.shape.replace(i, QPoint(centered.shape.at(i).x()-1, centered.shape.at(i).y()-1));
    }
    double averageX, averageY, max = 1.0;
    double tmpX = 0.0, tmpY = 0.0;
    for(int i = 0; i < centered.shape.size(); i++){
        tmpX += centered.shape.at(i).x();
        tmpY += centered.shape.at(i).y();
    }
    averageX = tmpX/centered.shape.size();
    averageY = tmpY/centered.shape.size();
    while(!((averageX > max|| averageX < -max)&&(averageY > max|| averageY < -max))){
        if(averageX > max){
            for(int i = 0; i < centered.shape.size(); i++){
                centered.shape.replace(i, QPoint(centered.shape.at(i).x()-1, centered.shape.at(i).y()));
            }
        }
        else if(averageX < -max){
            for(int i = 0; i < centered.shape.size(); i++){
                centered.shape.replace(i, QPoint(centered.shape.at(i).x()+1, centered.shape.at(i).y()));
            }
        }
        if(averageY > max){
            for(int i = 0; i < centered.shape.size(); i++){
                centered.shape.replace(i, QPoint(centered.shape.at(i).x(), centered.shape.at(i).y()-1));
            }
        }
        else if(averageY < -max){
            for(int i = 0; i < centered.shape.size(); i++){
                centered.shape.replace(i, QPoint(centered.shape.at(i).x(), centered.shape.at(i).y()+1));
            }
        }
        for(int i = 0; i < centered.shape.size(); i++){
            tmpX += centered.shape.at(i).x()-1.5;
            tmpY += centered.shape.at(i).y()-1.5;
        }
        averageX = tmpX/centered.shape.size();
        averageY = tmpY/centered.shape.size();
    }
    return centered;
}

bool Tetris::checkColissionRight(Block block)
{
    bool noColission = true;
    for(int i = 0; i < block.shape.size(); i++){
        QPoint tmp = block.shape.at(i);
        tmp.setX(tmp.x()+block.x);
        tmp.setY(tmp.y()+block.y);
        if(tmp.x()+1 > 13)
            noColission = false;
        else{
            for(int k = 0; k < scene.landed.size(); k++){
                Tile tile = scene.landed.at(k);
                if((tmp.y() == tile.y)&&(tmp.x()+1 == tile.x))
                    noColission = false;
            }
        }
    }
    return noColission;
}

bool Tetris::checkColissionLeft(Block block)
{
    bool noColission = true;
    for(int i = 0; i < block.shape.size(); i++){
        QPoint tmp = block.shape.at(i);
        tmp.setX(tmp.x()+block.x);
        tmp.setY(tmp.y()+block.y);
        if(tmp.x()-1 < 0)
            noColission = false;
        else{
            for(int k = 0; k < scene.landed.size(); k++){
                Tile tile = scene.landed.at(k);
                if((tmp.y() == tile.y)&&(tmp.x()-1 == tile.x))
                    noColission = false;
            }
        }
    }
    return noColission;
}

bool Tetris::checkColissionBottom(Block block)
{
    bool noColission = true;
    for(int i = 0; i < block.shape.size(); i++){
        QPoint tmp = block.shape.at(i);
        tmp.setX(tmp.x()+block.x);
        tmp.setY(tmp.y()+block.y);
        if(tmp.y()+1 > 19)
            noColission = false;
        else{
            for(int k = 0; k < scene.landed.size(); k++){
                Tile tile = scene.landed.at(k);
                if((tmp.x() == tile.x)&&(tmp.y()+1 == tile.y))
                    noColission = false;
            }
        }
    }
    return noColission;
}

void Tetris::nextBlock()
{
    blockToTiles();
    int rowsAtOnce = 0;
    score += 5*scoreMult;
    for(int i = 0; i < FIELD_HEIGHT; i++){
        QList<int> tmpList;
        for(int k = 0 ; k < scene.landed.size(); k++){
            if(scene.landed.at(k).y == i)
                tmpList.append(k);
        }
        if(tmpList.size()==FIELD_WIDTH){
            for(int k = 13; k > -1; k--){
                scene.landed.removeAt(tmpList.at(k));
            }
            for(int k = 0; k < scene.landed.size(); k++){
                Tile tmp = scene.landed.at(k);
                if((tmp.y) < i)
                    tmp.y +=1;
                scene.landed.replace(k, tmp);
            }
            rowsCompleted +=1;
            rowsAtOnce +=1;
            score += 50*scoreMult*rowsAtOnce;
            nextLevel();
            ui->rowsLabel->setText(QString("Zeilen:\n"+QString::number(rowsCompleted)));
        }
    }
    ui->scoreLabel->setText("score:\n"+QString::number(score));
    if(checkColissionBottom(scene.next)){
        scene.current = scene.next;
        scene.next = blocks.at(rand()%(blocks.size()));
        if(rand()%2 == 1)
            scene.next = rotateBlock(scene.next);
        renderNext();
    }
    else{
        //moveTimer->stop();
        scene.landed.clear();
        rowsCompleted = 0;
        level = 0;
        nextLevel();
        ui->rowsLabel->setText(QString("Zeilen:\n"+QString::number(rowsCompleted)));
    }
}

void Tetris::loadMods(QString filename)
{
    qDebug()<<"loading:"<<filename;
    QFile file(filename);
    file.open(QIODevice::ReadOnly);
    QByteArray data = file.readAll();
    xmlReader = new QXmlStreamReader(data);
    xmlReader->readNext();
    xmlReader->readNext();
    while(!xmlReader->hasError()){
        xmlReader->readNextStartElement();
        if(xmlReader->name() == "block"){
            Block current;
            xmlReader->readNextStartElement();
            if(xmlReader->name() == "texture"){
                QImage texture = QImage(xmlReader->attributes().last().value().toString());
                QColor color = QColor(xmlReader->attributes().first().value().toString());
                current.texture = colorGrayscale(texture, color).scaled(TILE_RES,TILE_RES);
            }
            xmlReader->readNextStartElement();
            xmlReader->readNextStartElement();
            if(xmlReader->name() == "shape"){
                QString shape = xmlReader->attributes().first().value().toString();
                QList<QString> tmpList = shape.split("|");
                for(int i = 0; i < tmpList.size(); i++){
                    QString tmp = tmpList.at(i);
                    tmp.remove("(");
                    tmp.remove(")");
                    QPoint point;
                    point.setX(tmp.split(",").first().toInt());
                    point.setY(tmp.split(",").last().toInt());
                    current.shape.append(point);
                }
            }
            current.x = 6;
            current.y = 0;
            blocks.append(current);
            xmlReader->readNextStartElement();
            xmlReader->readNextStartElement();
        }
        else if(xmlReader->name() == "level"){
            Level current;
            xmlReader->readNextStartElement();
            if(xmlReader->name() == "properties"){
                current.level = xmlReader->attributes().at(0).value().toInt();
                current.scoreMult = xmlReader->attributes().at(1).value().toInt();
                current.rowsNeeded = xmlReader->attributes().at(2).value().toInt();
                current.speed = xmlReader->attributes().at(3).value().toInt();
            }
            levels.append(current);
            xmlReader->readNextStartElement();
            xmlReader->readNextStartElement();
        }
        else if(xmlReader->name() == "testArea")
            break;
    }
    qDebug()<<"Levels:"<<levels.size()<<"Blöcke:"<<blocks.size();
}

void Tetris::nextLevel()
{
    for(int i = 0; i < levels.size(); i++){
        if((levels.at(i).level == level+1)&&(levels.at(i).rowsNeeded <= rowsCompleted)){
            scoreMult = levels.at(i).scoreMult;
            speed = levels.at(i).speed;
            moveTimer->setInterval(speed);
            level+=1;
            ui->levelLabel->setText(QString("Level:\n"+QString::number(level)));
        }
    }
}

#ifdef AI_ENABLED

void Tetris::action(double index)
{
    index *= 2.0;
    if(inGame && running){
        int i;
    if(index > 0 && index < 0.25)
        i = 1;
    else if( index >= 0.25 && index < 0.5)
        i = 2;
    else if( index >= 0.5 && index < 0.75)
        i = 3;
    else if( index >= 0.75 && index < 1.0)
        i = 4;
    else i = 5;
    switch(i){
    case  1:{
        if(checkColissionLeft(scene.current))
            scene.current.x -=1;
    }
    break;
    case 2 :{
        if(checkColissionRight(scene.current))
            scene.current.x +=1;
    }
    break;
    case 5 :{
        scene.current = rotateBlock(scene.current);
    }
    break;
    case 4 :{
        if(checkColissionBottom(scene.current))
            scene.current.y +=1;
        else
            nextBlock();
    }
    break;
    default :{
        ;
    }
    }
    renderScene();
    }
}

#endif

QImage Tetris::colorGrayscale(QImage old, QColor color)
{
    QImage final = QImage(old.width(),old.height(),QImage::Format_RGB888);
    for(int x = 0; x < final.width(); x++){
        for(int y = 0; y < final.height(); y++){
            QColor pixel;
            double ro, go, bo, rc, gc, bc, b;
            double red, green, blue;
            ro = old.pixelColor(x,y).red()/255.0;
            go = old.pixelColor(x,y).green()/255.0;
            bo = old.pixelColor(x,y).blue()/255.0;
            b = (ro+go+bo)/3;
            rc = color.red()/255.0;
            gc = color.green()/255.0;
            bc = color.blue()/255.0;
            red = (b*rc)*255.0;
            green = (b*gc)*255.0;
            blue = (b*bc)*255.0;
            pixel.setRed(red);
            pixel.setGreen(green);
            pixel.setBlue(blue);
            final.setPixelColor(x,y,pixel);
        }
    }
    return final;
}

void Tetris::keyPressEvent(QKeyEvent *event)
{
    if(inGame && running)
    switch(event->key()){
        case Qt::Key_Left:
        case Qt::Key_A :{
            if(checkColissionLeft(scene.current))
                scene.current.x -=1;
#ifdef AI_ENABLED
            ai.nextPass(getField(), QList<double>({0.1}));
#endif
        }
        break;
        case Qt::Key_Right:
        case Qt::Key_D :{
            if(checkColissionRight(scene.current))
                scene.current.x +=1;
#ifdef AI_ENABLED
            ai.nextPass(getField(), QList<double>({0.2}));
#endif
        }
        break;
    case Qt::Key_Up:
        case Qt::Key_W :{
            scene.current = rotateBlock(scene.current);
#ifdef AI_ENABLED
            ai.nextPass(getField(), QList<double>({0.3}));
#endif
        }
        break;
        case Qt::Key_Down:
        case Qt::Key_S :{
            if(checkColissionBottom(scene.current))
                scene.current.y +=1;
            else
                nextBlock();
#ifdef AI_ENABLED
            ai.nextPass(getField(), QList<double>({0.4}));
#endif
        }
        break;
        default :{
#ifdef AI_ENABLED
        ai.nextPass(getField(), QList<double>({0.9}));
#endif
            ;
        }
    }
    renderScene();
}

#ifdef AI_ENABLED

void Tetris::on_aiCheckBox_toggled(bool checked)
{
    AIenabled = checked;
}

#endif

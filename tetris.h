#ifndef TETRIS_H
#define TETRIS_H

//#define AI_ENABLED
#define TILE_RES 32     //resolution of the tiles
#define BLOCK_SIZE 4    //max size of a block
#define FIELD_WIDTH 14  //width of field
#define FIELD_HEIGHT 20 //height of field

#include <QTime>
#include <QTimer>
#include <QWidget>
#include <QPainter>
#include <QKeyEvent>
#include <QXmlStreamReader>
#include "soundsystem.h"

#ifdef AI_ENABLED
#include "ai.h"
#endif

enum Direction{
    turn_left,
    turn_right
};

struct Block{
    int x, y;
    QImage texture;
    QList<QPoint> shape;
};

struct Tile{
    int x, y;
    QImage texture;
};

struct Level{
    int level;      //index of level
    int scoreMult;  //multiplier for score
    int rowsNeeded; //completed rows needed to reach level
    int speed;      //speed in ms (time btw moves)
};

struct Scene{
    Block current;
    Block next;
    QList<Tile> landed;
};

//shapes:   {{0,0,0,0}      {{0,0,0,0}      {{0,0,0,0}
//          ,{0,1,1,0}      ,{1,1,0,0}      ,{0,0,1,1}
//          ,{0,1,1,0}      ,{0,1,1,0}      ,{0,1,1,0}
//          ,{0,0,0,0}}     ,{0,0,0,0}}     ,{0,0,0,0}}
//
//          {{0,1,0,0}      {{0,0,0,0}      {{0,1,1,0}
//          ,{0,1,0,0}      ,{0,1,1,1}      ,{0,1,0,0}
//          ,{0,1,0,0}      ,{0,0,1,0}      ,{0,1,0,0}
//          ,{0,1,0,0}}     ,{0,0,0,0}}     ,{0,0,0,0}}
//
//          {{0,1,1,0}
//          ,{0,0,1,0}
//          ,{0,0,1,0}
//          ,{0,0,0,0}}

namespace Ui {
class Tetris;
}

class Tetris : public QWidget
{
    Q_OBJECT

public:
    explicit Tetris(QWidget *parent = 0);
    ~Tetris();

public slots:
    void on_startButton_clicked();
    void on_pauseButton_clicked();
    void on_saveButton_clicked();
    void on_loadButton_clicked();
    void setupBlocks();
    void renderScene();
    void renderNext();
    void blockToTiles();    //convert current block to tiles
    Block rotateBlock(Block block);
    Block centerBlock(Block block);
    bool checkColissionRight(Block block);
    bool checkColissionLeft(Block block);
    bool checkColissionBottom(Block block);
    void nextBlock();
    void loadMods(QString filename);
    void nextLevel();
#ifdef AI_ENABLED
    void action(double index);
    QList<double> getField();
    void on_aiCheckBox_toggled(bool checked);
#endif

private:
    Ui::Tetris *ui;
    SoundSystem *soundSystem;
    bool inGame, running, AIenabled;
    QList<Block> blocks;
    QList<Level> levels;
    Scene scene;
    QTimer *moveTimer;
    QXmlStreamReader *xmlReader;
    int score, rowsCompleted, level, speed, scoreMult;

#ifdef AI_ENABLED
    AI ai;
#endif

    QImage colorGrayscale(QImage old, QColor color), nextImg, mainGame;
    void keyPressEvent(QKeyEvent *event);
};

#endif // TETRIS_H

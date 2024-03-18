#include<bits/stdc++.h>
using namespace std;
#include<graphics.h>
#include"Block.hpp"
#include<conio.h>

#define MAX_LEVEL 5
#define SCORE_FILE "score.txt"

const int SPEED_NORMAL[MAX_LEVEL] = {500,400,300,200,100}; //普通速度 
const int SPEED_QUICK = 40;   //快速降落速度

class Tetris
{
private:
    int delay;//设置降落的速度（依靠刷新）
    bool update;//是否更新画面的标志
    //0：空白，没有任何方块
    //>0:表示不同的方块类型
    vector<vector<int>> map;//地图
    int rows;
    int cols;
    int left;
    int top;
    int blockSize;
    IMAGE imgBG;
    Block* curBlock;
    Block* nextBlock;//预告方块
    Block bakBlock;//当前方块降落过程中，用来备份上一个合法位置
    int score;//当前的得分
    int max_score;//最高分
    int level;//当前关卡
    int lineCount;//当前已经消除了多少行
    bool gameOver;//游戏是否结束

    IMAGE imgOver;

private:
    void keyEvent();
    void updateWindows();
    int getDelay();
    void drop();//下落
    void clearLine();//清行
    void moveLeftRight(int offset);//左右移动
    void rotate();//旋转
    void drawScore();//绘制分数
    void checkOver();//检查游戏是否结束
    void saveScore();//保存最高分
    void displayOver();//绘制游戏结束画面
    
public:
    Tetris(int rows,int cols,int left,int top,int block_size);
    ~Tetris();
    void init();//初始化
    void play();//开始游戏
 };

Tetris::Tetris(int rows,int cols,int left,int top,int block_size){
    this->rows = rows;
    this->cols = cols;
    this->left = left;
    this->top = top;
    this->blockSize = block_size;
    for(int i = 0;i<rows;i++){
        vector<int> temp;
        for(int j = 0;j<cols;j++){
            temp.push_back(0);
        }
        this->map.push_back(temp);
    }
}

void Tetris:: play(){
    init();
    srand(time(NULL));

    nextBlock = new Block;
    curBlock = nextBlock;
    nextBlock = new Block;
    int time = 0;
    while(1){
        keyEvent();

        time+= getDelay();
        if(time>delay){
            time = 0;

            drop();
            update = true;
        }
        if(update){
            updateWindows();
            clearLine();
        }
        if(gameOver){
            //保存最高分
            saveScore();

            //更新游戏结束界面
            displayOver();

            system("pause");
            init();//重新开局
        }
        
    }
}

void Tetris::init(){
    delay = SPEED_NORMAL[0];

    srand(time(NULL));//随机数种子
    initgraph(938,896);//创建游戏窗口
    loadimage(&imgBG,"res/bg2.png");//加载背景图片
    loadimage(&imgOver,"res/over.png");//加载失败图片

    //初始化数据
    for (int i = 0; i < rows; i++) {
        for (int j = 0; j < cols; j++) {
            map[i][j] = 0;
        }
    }

    score = 0;
    level = 1;
    lineCount = 0;
    gameOver = false;
    //初始化最高分
    ifstream file(SCORE_FILE);
    if(!file.is_open()){
        max_score = 0;
        cout<<SCORE_FILE<<"打开失败"<<endl;
    }else{
        file>>max_score;
    }
    file.close();//文件关闭
}

void Tetris::keyEvent(){
    unsigned char ch;
    bool rotateFlag;
    int dx = 0;
    if(_kbhit()){
        ch = _getch();

        // 如果按下方向按键，会自动返回两个字符
        // 如果按下 向上方向键，会先后返回:224 72
        // 如果按下 向下方向键，会先后返回:224 80
        // 如果按下 向左方向键，会先后返回:224 75
        // 如果按下 向右方向键，会先后返回:224 77

        if(ch == 224){
            ch = _getch();
            switch (ch)
            {
            case 72://↑
                rotateFlag = true;
                break;
            case 80://↓
                delay = SPEED_QUICK;
                break;
            case 75://←
                dx = -1;
                break;
            case 77://→
                dx = 1;
                break;
            default:
                break;
            }
        }
    if (dx != 0) { //左右移动
		moveLeftRight(dx);
		update = true;
	}
 
	if (rotateFlag) {
		rotate();
		update = true;
	}
    }
}

void Tetris::updateWindows(){
    BeginBatchDraw();

    putimage(0,0,&imgBG);//绘制背景图片

    IMAGE** imgs = Block::getImages();
    for(int i = 0;i<rows;i++){
        for(int j = 0;j<cols;j++){
            if(map[i][j] == 0)
                continue;
            int x = j*blockSize+left;
            int y = i*blockSize+top;
            putimage(x,y,imgs[map[i][j]-1]);
        }
    }

    curBlock->draw(left,top);
    nextBlock->draw(left+427,top);

    //绘制分数
    drawScore();

    EndBatchDraw();

}

//第一次调用返回0，之后调用返回与上一次调用的时间差（ms）
int Tetris::getDelay(){
    static long long lastTime = 0;
    unsigned long long currentTime = GetTickCount();

    if(lastTime == 0){
        lastTime = currentTime;
        return 0;
    }else{
        int temp = currentTime - lastTime;
        lastTime = currentTime;
        return temp;
    }
}

void Tetris::drop(){
    bakBlock = *curBlock;

    curBlock->drop();
    if(!curBlock->blockInMap(map)){
        //将方块固化
        bakBlock.solidify(map);
        delete curBlock;
        curBlock = nextBlock;
        nextBlock = new Block;

        checkOver();
    }
    delay = SPEED_NORMAL[level-1];//防止方块一直快速下落，配合↓键的
}

void Tetris::moveLeftRight(int offset){
    bakBlock = *curBlock;
    curBlock->move_leftorright(offset);
    if(!curBlock->blockInMap(map)){
        *curBlock = bakBlock;
    }
}

void Tetris::rotate(){//旋转
    if(curBlock->getBlockType() == 7)
        return;

    bakBlock = *curBlock;
    curBlock->rotate();
    if(!curBlock->blockInMap(map)){
        *curBlock = bakBlock;
    }
}

void Tetris::clearLine(){
    int k = rows-1;
    int line = 0;
    for(int i = rows-1;i>=0;i--){
        int count = 0;
        for(int j = 0;j<cols;j++){
            if(map[i][j]){
                count++;
            }
            map[k][j] = map[i][j];
        }
        if(count<cols){
            k--;
        }else{
            line++;
        }
    }
    if(line>0){
        //计算得分
        int addScore[4] = {10,30,60,100};
        score += addScore[line-1];
        update = true;

        //每一百分一个级别
        level = (score+99)/100;
        lineCount+=line;
    }
}

void Tetris::drawScore(){
    char scoreText[32];
    sprintf_s(scoreText,sizeof(scoreText),"%d",score);

    setcolor(RGB(180,180,180));
    LOGFONT f;
    gettextstyle(&f);//获取当前字体
    f.lfHeight = 60;
    f.lfWidth = 30;
    f.lfQuality = ANTIALIASED_QUALITY;//打磨字体 使字体变得更加光滑
    strcpy_s(f.lfFaceName,sizeof(f.lfFaceName),_T("Segoe UI Black"));
    settextstyle(&f);
    setbkmode(TRANSPARENT);//字体的背景设置为透明效果
    outtextxy(670,727,scoreText);

    //绘制当前行数
    sprintf_s(scoreText,sizeof(scoreText),"%d",lineCount);
    gettextstyle(&f);
    int xPos = 224-f.lfWidth * strlen(scoreText);
    outtextxy(xPos,817,scoreText);

    //绘制当前是第几关
    sprintf_s(scoreText,sizeof(scoreText),"%d",level);
    outtextxy(224-30,727,scoreText);

    //绘制最高分
    sprintf_s(scoreText,sizeof(scoreText),"%d",max_score);
    outtextxy(670,817,scoreText);
}

void Tetris::checkOver(){   //检查游戏是否结束
    gameOver = (curBlock->blockInMap(map) == false);
}

void Tetris::saveScore(){   //保存最高分
    if(score>max_score){
        max_score = score;

        ofstream file(SCORE_FILE);
        file<<max_score;
        file.close();
    }
}

void Tetris::displayOver(){
    putimage(262,361,&imgOver);
}

Tetris::~Tetris(){

};
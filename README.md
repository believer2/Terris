### C++游戏：俄罗斯方块

#### 1. 概述

​		俄罗斯方块是我们非常熟悉的游戏，本文就是使用C++面向对象思想开发俄罗斯方块游戏，跟着B站up主[CPP编程学习](https://b23.tv/shlsfvd)，以及他写的[CSDN博客](https://blog.csdn.net/pzjdoytt/article/details/125325753)，学习后自己总结，方便之后复习。

#### 2.模块设计

游戏所有代码均在github，主要有三部分组成：

（1）main.cpp：主程序，启动游戏，引用Tetris.hpp

（2）Tetris.hpp:游戏主体类，私有变量有地图数据（存储方块的数据），游戏分数等；私有函数有控制方块下落、旋转、绘制分数等；公共函数有初始化和开始游戏。引用Block.hpp

（3）Block.hpp:方块类，生成七种方块，为游戏类提供方块的旋转，移动，下降等功能。

同时，由于需要绘图，所以需要引用easyx库，可以上网查询vscode如何[配置easyx](https://www.cnblogs.com/riskmoumou/p/18023837/extended-knowledge-vscode-configuration-easyx-brbl9).

#### 3.启动游戏

```c++
int main(){

  Tetris game(20,10,263,133,36);

  game.play();

  return 0;

}
```

主要是创建游戏类并运行游戏。而Tetris的构造函数如下：

```c++
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
```

![image-20240318162418442](D:\BaiduNetdiskDownload\提高编程能力资料\讲义\assets\image-20240318162418442.png)

row代表方块的行数，cols代表方块列数。

#### 4.实现游戏的主体架构

游戏最主要的逻辑如下：

```c++
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
```

首先是初始化，然后生成两个方块，在游戏中我们都知道，一个是当前的方块，而另一个是预告方块。

游戏中，最主要是每隔一段时间，更新一下游戏画面，所以需要getDelay()函数，调用返回与上一次调用的时间差，而delay正好可以控制刷新的间隔，就可以控制使方块下降的更快；同时增加一个是否更新的布尔值（update），是因为不止随着时间才需要更新当前画面，还有移动、旋转、消行等操作都需要立马更新游戏画面 ，那么这些操作只需要将update置为true即可。这就是游戏最主体的部分。

#### 5.创建方块

游戏主体已经有了的话，目前最重要的就是，我们没有方块啊，所以需要在方块类中进行创建。

在调用 new Block 时，会自动调用Block的默认构造函数，所以我们需要在这个构造函数里面完成新方块的创建。俄罗斯方块的表示方法有很多，最常见的是使用一个二维数组，表示一种俄罗斯方块的某种形态，也就是说，一个俄罗斯方块，需要使用4个二维数组来表示各个形态（4个方向），不过就比较麻烦，所以就采取下面的方法进行存储

![image-20240318163921177](D:\BaiduNetdiskDownload\提高编程能力资料\讲义\assets\image-20240318163921177.png)

所以初始化就如下：

```c++
Block::Block(){
    if (imgs[0] == NULL) {
		IMAGE imgTmp;
		loadimage(&imgTmp, "res/tiles.png");
		SetWorkingImage(&imgTmp);
		for (int i = 0; i < 7; i++) {
			imgs[i] = new IMAGE;
			getimage(imgs[i], i * size, 0, size, size);
		}
		SetWorkingImage();//恢复工作区
		srand(time(NULL));
	}
    int blocks[7][4] = {
		1,3,5,7, // I
		2,4,5,7, // Z 1型
		3,5,4,6, // Z 2型
		3,5,4,7, // T
		2,3,5,7, // L
		3,5,7,6, // J
		2,3,4,5, // 田
	};
    blockType = rand()%7+1;
    for(int i = 0;i<4;i++){
        blockLocation[i].row = blocks[blockType-1][i]/2;
        blockLocation[i].col = blocks[blockType-1][i]%2;
    }
    img = imgs[blockType-1];
}
```

我们使用下面的方块来组成七种方块的颜色，并用1~7代表七种方块的类型（blockType），并将方块初始化。

![tiles](D:\BaiduNetdiskDownload\提高编程能力资料\讲义\assets\tiles.png)

```c++
struct Point {
	int row;
	int col;
};
```

同时，在每块方块的私有变量中，有  Point blockLocation[4]，存储了当前方块在地图中的位置，第几行第几列（地图之前初始化为20行10列）。

所以，方块的下降函数以及绘制函数就较为简单了：

```c++
void Block::drop(){
    for(int i = 0;i<4;i++){
        blockLocation[i].row++;
    }
}

void Block::draw(int left,int top){
    for(int i = 0;i<4;i++){
        int x = left + blockLocation[i].col*size;
        int y = top + blockLocation[i].row*size;
        putimage(x,y,img);
    }
}
```

非常简单，但是想想，如果方块运行起来的话，就会一直下降，没有边界的下降，并不符合我们所想的，所以后面需要解决，一步一步来。

#### 6.游戏场景初始化

在4中，我们创建了游戏运行的基本框架，如果函数都有的话，我们任务就完成了。而游戏运行的第一步，就是最重要的，游戏的初始化。

首先我们肯定想着把前面展示的背景初始化加载一下，然后呢，我们游戏类中的私有变量都需要初始化，比如二维地图数组，都要初始化为0（0代表没有方块，而其他数字1~7代表着七种方块，这也是之前方块类型blockType是从1~7，而不是0~6的原因）；私有变量还有分数，关卡等级等等。同时游戏存在着之前游玩的最高分，存放在游戏同一目录下的score.txt中，需要读取出来，初始化最高分（没有最高分，如何激励玩家冲击更高的分数呢）。理清楚初始化后，代码就显而易见了：

```c++
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
```

这样的话，我们游戏就有背景了！但是，没有方块怎么行，下一节就开始画方块喽。

#### 7.方块的渲染

我们想让方块在游戏类中呈现，是不是还需要方块类中的图像数据，所以需要增加获取各种方块的图形纹理的函数：

```c++
//Block.hpp
class Block {
public:
    static IMAGE** getImages();
    ......
}
//Block.hpp
IMAGE** Block::getImages()
{
	return imgs;
}
```

在游戏主体中，绘制背景图片和方块，都在updateWindows（）中，绘制也很简单，就是在指定的位置放上刚才获取的方块图片就行了，方块数据都存在map中。

```c++
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
```

方块在游戏类中有了，如果我们现在运行的话，是不是方块就在顶部，它根本就不动啊（突然想起来，方块类之前写了降落是不是），但是存在着两个问题：

（1）方块不能一直下落，我们得知道他能不能再下落了（设计了blockInMap()函数）

（2）方块不能下落后，我们是不是要把方块“固化”，要不然它降落到不能再下落后，就消失了（设计了solidify（）函数）

所以，在游戏类中，方块的降落函数，就有了：

```c++
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
```

下落的时候，是不是得之前下落之前的状态，下落后“不合法”，我们就把之前的状态给“固化”，就可以了吧。

所以需要补充函数：

（1）补充实现Block的赋值构造函数(防止[浅拷贝问题](https://blog.csdn.net/crr411422/article/details/131033612)):

```c++
Block& Block::operator=(const Block& other)
{
	if (this == &other) return *this;
 
	this->blockType = other.blockType;
	for (int i = 0; i < 4; i++) {
		this->smallBlocks[i] = other.smallBlocks[i];
	}
 
	return *this;
}
```

（2）判断能否继续下落（在地图里面，就可以继续下落）：

```c++
bool Block::blockInMap(const vector<vector<int>> &map){
    int rows = map.size();
    int cols = map[0].size();
    for(int i = 0;i<4;i++){
        if(blockLocation[i].col<0 || blockLocation[i].col>=cols ||
           blockLocation[i].row<0 || blockLocation[i].row>=rows ||
           map[blockLocation[i].row][blockLocation[i].col]){
            return false;
           }
    }
    return true;
}
```

（3）将方块“固化”（就是将方块的类型写入map中就行了，所以需要传入map）：

```c++
void Block::solidify(vector<vector<int>> &map){
    for(int i =0;i<4;i++){
        map[blockLocation[i].row][blockLocation[i].col] = blockType;
    }
}
```

#### 8.方块的左右移动、旋转

在上一节我们实现了方块在游戏类中的下落（绘制也在上一节），那么我们也可以继续处理方块的左右移动和旋转了。在游戏主题中，我们已经预留了函数keyEvent()，这就是我们用来处理左右移动和旋转的函数。

左右移动我们想想就很简单，只要别越界出了地图就行，而旋转，则更为麻烦，我们需要想到，如何将原本方块的位置信息和旋转后的方块的位置信息进行对应，这就需要数学方法了，在代码中可以体现，可以试一试看看怎么推出来。哦对，还有按下键，可以让方块更快的下落，只需要改一下delay的数值就行。（之前的drop，还有还原delay的操作，要不然，按一次↓键，方块就会一直快速下落，这与我们本来的打算不符合）

```c++
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
```

每次处理移动和旋转后，就需要将update更新为true，立刻更新画面。

所以需要补充函数：

（1）左右移动函数：

```c++
void Tetris::moveLeftRight(int offset){
    bakBlock = *curBlock;
    curBlock->move_leftorright(offset);
    if(!curBlock->blockInMap(map)){
        *curBlock = bakBlock;
    }
}
```

这里还需继续调用方块类中的移动函数：

```c++
void Block::move_leftorright(int offset){    //左右移动
    for (int i = 0; i < 4; i++) {
		blockLocation[i].col += offset;
	}
}
```

（2）旋转函数：

```c++
void Tetris::rotate(){//旋转
    if(curBlock->getBlockType() == 7)
        return;

    bakBlock = *curBlock;
    curBlock->rotate();
    if(!curBlock->blockInMap(map)){
        *curBlock = bakBlock;
    }
}
```

同样的，这里也需要调用方块类中的旋转函数（因为方块的数据都在自己类中，还是需要到 方块类中操作）：

```c++
void Block::rotate(){//旋转
    Point p = blockLocation[1];
    for(int i = 0;i<4;i++){
        Point temp = blockLocation[i];
        blockLocation[i].row = p.row + temp.col - p.col;
        blockLocation[i].col = p.col - temp.row + p.row;
    }
}
```

这样的话，方块就实现了旋转（↑），左右移动（←→），以及快速下落（↓），那么我们接下来就是需要判断消行喽。

#### 9.消行

消行，其实也没有那么难，就是一行全不为0，就可以消除了。这在游戏主体中也是预留的函数的：clearLine()。

```c++
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
```

消除也需要将上一个行的数据移动到下一行，然后根据消除的行数，计算得分。对，说到得分也需要将目前的得分、最高分、当前消除的行数，绘制到背景上才行，这在游戏主体中也预留了函数drawScore():

```c++
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
```

当然，也会有判断游戏是否结束，很简单，当前生成的方块直接就“非法”，说明就要结束了。游戏结束后，将最高分保存，写入文件即可。



整个游戏到这里也就结束了，完结撒花!❀


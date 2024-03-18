#include<bits/stdc++.h>
using namespace std;
#include<graphics.h>


struct Point {
	int row;
	int col;
};
class Block
{
private:
    int blockType;
    Point blockLocation[4];
    IMAGE *img;
    static IMAGE* imgs[7];
    static int size;
public:
    Block(/* args */);
    ~Block();
    void drop();//下降
    void move_leftorright(int offset);//左右移动
    void rotate();//旋转
    void draw(int left,int top);
    static IMAGE** getImages();
    Block& operator=(const Block& other);
    bool blockInMap(const vector<vector<int>> &map);
    void solidify(vector<vector<int>> &map);//“固化”
    int getBlockType();
};

Block& Block::operator=(const Block& other)
{
	if (this == &other) return *this;
 
	this->blockType = other.blockType;
	for (int i = 0; i < 4; i++) {
		this->blockLocation[i] = other.blockLocation[i];
	}
 
	return *this;
}


IMAGE* Block::imgs[7] = {NULL};
int Block::size = 36;

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

void Block::solidify(vector<vector<int>> &map){
    for(int i =0;i<4;i++){
        map[blockLocation[i].row][blockLocation[i].col] = blockType;
    }
}

void Block::move_leftorright(int offset){    //左右移动
    for (int i = 0; i < 4; i++) {
		blockLocation[i].col += offset;
	}
}

void Block::rotate(){//旋转
    Point p = blockLocation[1];

    for(int i = 0;i<4;i++){
        Point temp = blockLocation[i];
        blockLocation[i].row = p.row + temp.col - p.col;
        blockLocation[i].col = p.col - temp.row + p.row;
    }
}

int Block::getBlockType(){
    return blockType;
}

IMAGE** Block::getImages(){
    return imgs;
}

Block::~Block(){

}
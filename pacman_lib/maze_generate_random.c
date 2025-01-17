#include "maze_generate_random.h"
#include "maze.h"           // Chứa Maze, ROOM_TYP_..., ROOM_CNT_X, ROOM_CNT_Y,...
#include "random.h"         // random_init(), get_randrange(), get_random_bit()
#include "stm32_ub_font.h"  // Nếu bạn cần debug in ra

#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>   // malloc, free
#include <string.h>   // memset
#include <stdio.h>    // sprintf (nếu cần debug)

//------------------------------------------------------------------------------
// 1) Các hằng số & cấu trúc “logic” theo code JS
//------------------------------------------------------------------------------
#define UP    0
#define RIGHT 1
#define DOWN  2
#define LEFT  3

// Lưới logic cỡ 9×5 (giống code JS)
static const int LOGIC_ROWS = 9;
static const int LOGIC_COLS = 5;

// Cấu trúc Cell (1 ô logic 9×5)
typedef struct {
    int x, y;
    bool filled;
    bool connect[4];        // connect[UP], connect[RIGHT], connect[DOWN], connect[LEFT]
    struct Cell_s* next[4]; // trỏ sang cell láng giềng
    int no;
    int group;
    bool isRaiseHeightCandidate;
    bool isShrinkWidthCandidate;
    bool isJoinCandidate;
    bool isSingleDeadEndCandidate;
    bool isDoubleDeadEndCandidate;
    bool isVoidTunnelCandidate;
    bool isEdgeTunnelCandidate;
    bool topTunnel;
} Cell_s;

// Mảng logic
static Cell_s g_cells[9*5]; // 9*5=45 cell
// Mảng theo code JS
static int tallRows[9];    // cẩn thận với size; code gốc JS hẳn “có logic phức tạp”
static int narrowCols[9];  // tuỳ, ta đặt 9 cho an toàn

//------------------------------------------------------------------------------
// 2) Hàm random tiện ích (thay Math.random())
//------------------------------------------------------------------------------
static int getRandomInt(int min, int max)
{
    // Dùng hàm do bạn cung cấp
    return (int)get_randrange((uint32_t)min, (uint32_t)max);
}

// Lấy ngẫu nhiên float [0..1], bằng cách get_randrange(0,100)/100.0
static float getRandomFloat01(void)
{
    int r = getRandomInt(0, 100);
    return (float)r / 100.0f;
}

// Đổi chỗ 2 int
static void swap_int(int *a, int *b)
{
    int tmp = *a;
    *a = *b;
    *b = tmp;
}

// Shuffle 1 mảng int (Fisher-Yates)
static void shuffle_array(int *arr, int len)
{
    int i;
    for(i=0; i<len; i++){
        int j = getRandomInt(0, len-1);
        swap_int(&arr[i], &arr[j]);
    }
}

// Lấy 1 phần tử random
static int randomElement(const int *arr, int len)
{
    if(len<=0) return -1;
    int idx = getRandomInt(0, len-1);
    return arr[idx];
}

//------------------------------------------------------------------------------
// 3) resetCells() – khởi tạo lưới 9×5
//------------------------------------------------------------------------------
static void resetCells(void)
{
    int i;
    for(i=0; i<LOGIC_ROWS*LOGIC_COLS; i++){
        g_cells[i].x = i % LOGIC_COLS;
        g_cells[i].y = i / LOGIC_COLS;
        g_cells[i].filled = false;
        for(int d=0; d<4; d++){
            g_cells[i].connect[d] = false;
            g_cells[i].next[d] = NULL;
        }
        g_cells[i].no = -1;
        g_cells[i].group = -1;
        g_cells[i].isRaiseHeightCandidate = false;
        g_cells[i].isShrinkWidthCandidate = false;
        g_cells[i].isJoinCandidate = false;
        g_cells[i].isSingleDeadEndCandidate = false;
        g_cells[i].isDoubleDeadEndCandidate = false;
        g_cells[i].isVoidTunnelCandidate = false;
        g_cells[i].isEdgeTunnelCandidate = false;
        g_cells[i].topTunnel = false;
    }

    // Gán next[] 4 hướng
    for(i=0; i<LOGIC_ROWS*LOGIC_COLS; i++){
        Cell_s* c = &g_cells[i];
        int x = c->x;
        int y = c->y;
        if(x>0)                c->next[LEFT]  = (struct Cell_s*)&g_cells[i-1];
        if(x<LOGIC_COLS-1)     c->next[RIGHT] = (struct Cell_s*)&g_cells[i+1];
        if(y>0)                c->next[UP]    = (struct Cell_s*)&g_cells[i-LOGIC_COLS];
        if(y<LOGIC_ROWS-1)     c->next[DOWN]  = (struct Cell_s*)&g_cells[i+LOGIC_COLS];
    }

    // Định nghĩa ghost home 4 ô “ở giữa” (giống JS code)
    // i=3*cols => y=3, x=0
    {
        int idx = 3*LOGIC_COLS; 
        g_cells[idx].filled = true;
        g_cells[idx].connect[LEFT] = true;
        g_cells[idx].connect[RIGHT] = true;
        g_cells[idx].connect[DOWN] = true;

        idx++;
        g_cells[idx].filled = true;
        g_cells[idx].connect[LEFT] = true;
        g_cells[idx].connect[DOWN] = true;

        idx += (LOGIC_COLS - 1);
        g_cells[idx].filled = true;
        g_cells[idx].connect[LEFT] = true;
        g_cells[idx].connect[UP] = true;
        g_cells[idx].connect[RIGHT] = true;

        idx++;
        g_cells[idx].filled = true;
        g_cells[idx].connect[UP] = true;
        g_cells[idx].connect[LEFT] = true;
    }
}

//------------------------------------------------------------------------------
// Hàm fillCell() – đánh dấu 1 cell
//------------------------------------------------------------------------------
static void fillCell(Cell_s *c, int groupID, int *pCellCount)
{
    c->filled = true;
    c->no = (*pCellCount);
    (*pCellCount)++;
    c->group = groupID;
}

//------------------------------------------------------------------------------
// genRandom() – tương đương code JS (rút gọn)
//------------------------------------------------------------------------------
static void setResizeCandidates(void);
static bool createTunnels(void);
static bool chooseTallRows(void);
static bool chooseNarrowCols(void);
static bool isDesirable(void);

static void genRandom(void)
{
    resetCells();

    int numFilled=0;   // đếm cell
    int numGroups=0;   // đếm group

    // Xác suất dừng
    float probStopGrowingAtSize[6] = {
        0.0f,   // size=0
        0.0f,   // size=1
        0.10f,  // size=2
        0.50f,  // size=3
        0.75f,  // size=4
        1.00f   // size=5
    };

    int singleCountTop=0;
    int singleCountBottom=0;
    float probTopAndBotSingleCellJoin = 0.35f;

    // Tạo group
    while(1){
        // Tìm các cell trống leftmost
        int arrLeft[64];
        int countLeft=0;
        int x,y;
        bool foundAny=false;
        for(x=0; x<LOGIC_COLS; x++){
            for(y=0; y<LOGIC_ROWS; y++){
                int idx = y*LOGIC_COLS + x;
                if(!g_cells[idx].filled){
                    arrLeft[countLeft++] = idx;
                }
            }
            if(countLeft>0){
                foundAny=true;
                break;
            }
        }
        if(!foundAny) {
            // ko còn cell trống
            break;
        }

        int iRand = getRandomInt(0, countLeft-1);
        int startIdx = arrLeft[iRand];
        Cell_s* startCell = &g_cells[startIdx];

        fillCell(startCell, numGroups, &numFilled);

        // random single cell (trên top/bottom)
        if(startCell->x < LOGIC_COLS-1){
            if(startCell->y==0 || startCell->y==(LOGIC_ROWS-1)){
                if(get_random_bit()){
                    if(getRandomFloat01()<=probTopAndBotSingleCellJoin){
                        if(startCell->y==0 && singleCountTop==0){
                            startCell->connect[UP] = true;
                            singleCountTop++;
                            numGroups++;
                            continue;
                        }
                        else if(startCell->y==(LOGIC_ROWS-1) && singleCountBottom==0){
                            startCell->connect[DOWN] = true;
                            singleCountBottom++;
                            numGroups++;
                            continue;
                        }
                    }
                }
            }
        }

        int size=1;
        int dirLast=-1;
        Cell_s* cur = startCell;

        while(size<5){
            bool stop=false;
            // Tìm các hướng open
            int dirs[4];
            int nDir=0;

            // Quét 4 hướng
            for(int d=0; d<4; d++){
                Cell_s* nC = (Cell_s*)cur->next[d];
                if(!nC) continue;
                if(!nC->filled){
                    // cấm cắt ghost?
                    if(!((cur->y==6 && cur->x==0 && d==DOWN) ||
                         (cur->y==7 && cur->x==0 && d==UP))){
                        dirs[nDir++] = d;
                    }
                }
            }
            if(nDir==0){
                stop=true;
            }
            else {
                // random 1 hướng
                int dd = dirs[getRandomInt(0,nDir-1)];
                cur->connect[dd] = true;
                Cell_s* nxt = (Cell_s*)cur->next[dd];
                nxt->connect[(dd+2)%4] = true;
                fillCell(nxt, numGroups, &numFilled);
                size++;
                dirLast=dd;
                cur=nxt;

                float rr = getRandomFloat01();
                if(rr <= probStopGrowingAtSize[size]){
                    stop=true;
                }
            }

            if(stop) {
                break;
            }
        }

        numGroups++;
    }

    // setResize
    setResizeCandidates();
}

//------------------------------------------------------------------------------
// genRandomFull() – lặp đến khi “isDesirable()” & “createTunnels()” ok
//------------------------------------------------------------------------------
static void genRandomFull(void)
{
    int genCount=0;
    while(1){
        genCount++;
        genRandom();
        if(!isDesirable()){
            continue;
        }
        setResizeCandidates();
        if(!createTunnels()){
            continue;
        }
        break;
    }
}

//------------------------------------------------------------------------------
// setResizeCandidates() – gắn cờ
//------------------------------------------------------------------------------
static void setResizeCandidates(void)
{
    int i;
    for(i=0; i<LOGIC_ROWS*LOGIC_COLS; i++){
        Cell_s* c = &g_cells[i];
        bool* q = c->connect;
        int x = c->x;
        int y = c->y;

        // Kiểu code JS
        if((x==0 || !q[LEFT]) &&
           (x==(LOGIC_COLS-1) || !q[RIGHT]) &&
           (q[UP] != q[DOWN])){
            c->isRaiseHeightCandidate = true;
        }

        Cell_s* c2 = (Cell_s*)c->next[RIGHT];
        if(c2){
            bool* q2 = c2->connect;
            if(((x==0||!q[LEFT]) && !q[UP] && !q[DOWN]) &&
               ((c2->x==(LOGIC_COLS-1)||!q2[RIGHT]) && !q2[UP] && !q2[DOWN])){
                c->isRaiseHeightCandidate = true;
                c2->isRaiseHeightCandidate = true;
            }
        }

        // flexible width
        if(x==(LOGIC_COLS-1) && q[RIGHT]){
            c->isShrinkWidthCandidate = true;
        }
        if((y==0 || !q[UP]) &&
           (y==(LOGIC_ROWS-1) || !q[DOWN]) &&
           (q[LEFT]!=q[RIGHT])){
            c->isShrinkWidthCandidate = true;
        }
    }
}

//------------------------------------------------------------------------------
// isDesirable()
static bool isDesirable(void)
{
    // Rút gọn => return true
    return true;
}

//------------------------------------------------------------------------------
// createTunnels()
static bool createTunnels(void)
{
    // Rút gọn => return true
    return true;
}

//------------------------------------------------------------------------------
// chooseTallRows() + chooseNarrowCols() – rút gọn => return true
static bool chooseTallRows(void)
{
    return true;
}
static bool chooseNarrowCols(void)
{
    return true;
}

//------------------------------------------------------------------------------
// getTiles() – giống JS, nhưng rút gọn
// Lấy subrows = rows*3+1+3 = 9*3+1+3=31
// fullcols = ...
// Mỗi tile là 1 ký tự .| o ...
//------------------------------------------------------------------------------
static char* getTiles(void)
{
    int subrows = LOGIC_ROWS*3 + 1 + 3; // 9*3+4=31
    int subcols = LOGIC_COLS*3 -1 +2;   // 5*3-1+2=15-1+2=16
    int fullcols = (subcols-2)*2;       // = (16-2)*2=14*2=28
    int totalSize = subrows * fullcols; // =31*28=868

    char* tiles = (char*)malloc(totalSize);
    if(!tiles) return NULL;
    memset(tiles, '_', totalSize);

    // Rút gọn: nếu filled => ghi '.'

    int i;
    for(i=0; i<LOGIC_ROWS*LOGIC_COLS; i++){
        if(!g_cells[i].filled) continue;
        // Tính toạ độ phóng đại: x0,y0
        int x0 = g_cells[i].x*3; 
        int y0 = g_cells[i].y*3; 
        // Từ code JS => index = ...
        // Ta rút gọn => check bounds
        int tileX = x0; 
        int tileY = y0; 
        // Tính offset
        // JS: px = tileX-2, idx=(midcols+px)+ tileY*fullcols => ...
        // Rút gọn => if inbound => tiles[idx]='.'
        int midcols = subcols - 2; // =14
        int px = tileX - 2;
        if(px<0) px=0;

        int idx = (midcols + px) + tileY*fullcols;
        if(idx>=0 && idx<totalSize){
            tiles[idx] = '.';
        }
    }
    return tiles;
}

//------------------------------------------------------------------------------
// maze_transform_tiles_to_maze() – từ tiles => Maze.Room
//------------------------------------------------------------------------------
static void maze_transform_tiles_to_maze(const char* tiles)
{
    int subrows = LOGIC_ROWS*3 +1 +3; //31
    int subcols = LOGIC_COLS*3 -1 +2; //16
    int fullcols = (subcols-2)*2;     //28

    // Maze => 28×31
    int mapW = 28;
    int mapH = 31;

    // Dọn
    for(int y=0; y<mapH; y++){
        for(int x=0; x<mapW; x++){
            Maze.Room[x][y].typ     = ROOM_TYP_WALL;
            Maze.Room[x][y].special = ROOM_SPEC_NONE;
            Maze.Room[x][y].door    = ROOM_DOOR_NONE;
            Maze.Room[x][y].skin    = ROOM_SKIN_WALL_STD;  
            Maze.Room[x][y].points  = ROOM_POINTS_NONE;
            Maze.Room[x][y].deb_err = ROOM_DEB_OK;
        }
    }

    // Gán PATH cho '.' (và có thể 'o','-',' ')
    for(int yy=0; yy<subrows && yy<mapH; yy++){
        for(int xx=0; xx<fullcols && xx<mapW; xx++){
            int idx = yy*fullcols + xx;
            char c = tiles[idx];
            if(c=='.' || c=='o' || c==' ' || c=='-'){
                Maze.Room[xx][yy].typ = ROOM_TYP_PATH;
                if(c=='o'){
                    Maze.Room[xx][yy].points = ROOM_POINTS_ENERGY;
                }
            }
        }
    }
}

//------------------------------------------------------------------------------
// 4) Hàm PUBLIC: maze_make_rooms_random()
//------------------------------------------------------------------------------
void maze_make_rooms_random(void)
{
    // 1) Khởi tạo RNG
    random_init();

    // 2) genRandomFull
    genRandomFull();

    // 3) getTiles
    char* tiles = getTiles();
    if(!tiles){
        // hết RAM
        return;
    }

    // 4) transform -> Maze.Room
    maze_transform_tiles_to_maze(tiles);

    // 5) Giải phóng
    free(tiles);

    // 6) Ví dụ: random 4 ô energy
    {
        int countEnergy=0;
        while(countEnergy<4){
            uint32_t rx = get_randrange(1, 26);
            uint32_t ry = get_randrange(1, 26);
            if(Maze.Room[rx][ry].typ==ROOM_TYP_PATH &&
               Maze.Room[rx][ry].points==ROOM_POINTS_NONE)
            {
                Maze.Room[rx][ry].points = ROOM_POINTS_ENERGY;
                countEnergy++;
            }
        }
    }

    // Từ đây Maze.Room[][] đã là bản đồ ngẫu nhiên.
    // Bạn có thể set skin, cổng gate, v.v. tuỳ ý.
}

/**************************************************************
 * File: maze_generate.c
 * Here we implement a random-based map generation 
 * rather than the old "fix-cứng" approach.
 * 
 * C89 style. 
 **************************************************************/

#include "maze_generate.h"
#include "random.h"   // random_init(), get_randrange(), get_random_bit()
#include "stm32_ub_font.h" // if needed for debug
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* 
 * We'll keep Maze global from maze.h:
 *   extern Maze_t Maze; 
 *
 * We'll create an internal function "maze_generate_reset_to_wall()"
 * that sets all rooms to WALL+std. Then "maze_generate_init_random_map()"
 * will "dig" random PATH using your random logic (like 9x5 => upscale => 28x31).
 */

/*----------------------------------------------------------------------
 * Helper: set all rooms to WALL with standard skin, no door, no points
 *---------------------------------------------------------------------*/
static void maze_generate_reset_to_wall(void)
{
    uint32_t x,y;
    for(y=0; y<ROOM_CNT_Y; y++){
        for(x=0; x<ROOM_CNT_X; x++){
            Maze.Room[x][y].typ     = ROOM_TYP_WALL;
            Maze.Room[x][y].special = ROOM_SPEC_NONE;
            Maze.Room[x][y].door    = ROOM_DOOR_NONE;
            Maze.Room[x][y].skin    = ROOM_SKIN_WALL_STD; 
            Maze.Room[x][y].points  = ROOM_POINTS_NONE;
            Maze.Room[x][y].deb_err = ROOM_DEB_OK;
        }
    }
}

/*----------------------------------------------------------------------
 * "maze_generate_init_random_map()"
 * Instead of "digpath_h/v" fix cứng, we do a random approach.
 * For example: create a logic 9x5 -> transform -> Maze. 
 * (similar to code we wrote in "maze_make_rooms_random")
 *---------------------------------------------------------------------*/
void maze_generate_init_random_map(void)
{
    /* 1) reset RNG */
    random_init();

    /* 2) set all rooms to WALL */
    maze_generate_reset_to_wall();

    /* 3) Ta có thể implement logic random 
     *    - Hoặc copy code "maze_make_rooms_random()" 
     *    - Hoặc 1 approach rút gọn. 
     */

    /* 
     * Ở đây, ta demo rút gọn: random "tạo đường" ngang/dọc 
     * Mỗi lần random x,y, random length, random vertical/horizontal. 
     * => Maze.Room[]-> PATH + door
     */
    {
        int i;
        int tries = 100; /* 100 random lines */
        for(i=0; i<tries; i++){
            uint32_t xx = get_randrange(1, ROOM_CNT_X-2);
            uint32_t yy = get_randrange(1, ROOM_CNT_Y-2);
            uint32_t len = get_randrange(2, 8);
            uint32_t p = (get_random_bit()) ? ROOM_POINTS_NORMAL : ROOM_POINTS_NONE;
            if(get_random_bit()){
                /* horizontal line */
                uint32_t ret = 0;
                uint32_t x2;
                if(xx+len >= ROOM_CNT_X) {
                    len = ROOM_CNT_X-1 - xx;
                }
                /* "dig" => set door flags + points */
                for(x2=xx; x2<xx+len; x2++){
                    if((x2-xx)==0){
                        Maze.Room[x2][yy].door |= ROOM_DOOR_R; 
                    }
                    else if((x2-xx)==(len-1)){
                        Maze.Room[x2][yy].door |= ROOM_DOOR_L;
                    }
                    else{
                        Maze.Room[x2][yy].door |= (ROOM_DOOR_L | ROOM_DOOR_R);
                    }
                    if(p!=ROOM_POINTS_NONE){
                        Maze.Room[x2][yy].points = p;
                    }
                }
            }
            else{
                /* vertical line */
                uint32_t y2;
                if(yy+len >= ROOM_CNT_Y) {
                    len = ROOM_CNT_Y-1 - yy;
                }
                for(y2=yy; y2<yy+len; y2++){
                    if((y2-yy)==0){
                        Maze.Room[xx][y2].door |= ROOM_DOOR_D; 
                    }
                    else if((y2-yy)==(len-1)){
                        Maze.Room[xx][y2].door |= ROOM_DOOR_U;
                    }
                    else{
                        Maze.Room[xx][y2].door |= (ROOM_DOOR_U | ROOM_DOOR_D);
                    }
                    if(p!=ROOM_POINTS_NONE){
                        Maze.Room[xx][y2].points = p;
                    }
                }
            }
        }
    }

    /* 4) Bây giờ set type=PATH cho room có door != 0
     *    => Sử dụng "maze_generate_searchandset(SEARCH_DOORS_YES,...)"
     */
    {
        Room_t room;
        room.typ     = ROOM_TYP_PATH;
        room.special = ROOM_SPEC_NONE;
        room.door    = ROOM_DOOR_NONE;  /* door mask is not overwritten */
        room.skin    = ROOM_SKIN_PATH_STD; 
        room.points  = ROOM_POINTS_NONE; /* we keep existing "points" */
        room.deb_err = ROOM_DEB_OK;
        maze_generate_searchandset(SEARCH_DOORS_YES, room);
    }

    /* 5) optionally: place 4 random energies */
    {
        int countE=0;
        while(countE<4){
            uint32_t rx= get_randrange(1, ROOM_CNT_X-2);
            uint32_t ry= get_randrange(1, ROOM_CNT_Y-2);
            if(Maze.Room[rx][ry].typ==ROOM_TYP_PATH &&
               Maze.Room[rx][ry].points==ROOM_POINTS_NONE)
            {
                Maze.Room[rx][ry].points= ROOM_POINTS_ENERGY;
                countE++;
            }
        }
    }
}

/*----------------------------------------------------------------------
 * maze_generate_searchandset()
 * Giữ nguyên code cũ, bạn đã đưa
 *---------------------------------------------------------------------*/
void maze_generate_searchandset(uint32_t mark, Room_t room)
{
    uint32_t x,y;
    for(y=0; y<ROOM_CNT_Y; y++){
        for(x=0; x<ROOM_CNT_X; x++){
            if(mark==SEARCH_ROOMS_ALL){
                /* set data of all rooms */
                Maze.Room[x][y].typ     = room.typ;
                Maze.Room[x][y].special = room.special;
                Maze.Room[x][y].door    = room.door;
                Maze.Room[x][y].skin    = room.skin;
                Maze.Room[x][y].points  = room.points;
                Maze.Room[x][y].deb_err = room.deb_err;
            }
            else if(mark==SEARCH_DOORS_YES){
                if(Maze.Room[x][y].door != ROOM_DOOR_NONE){
                    Maze.Room[x][y].typ  = room.typ;
                    Maze.Room[x][y].skin = room.skin;
                }
            }
            else if(mark==SEARCH_SKIN_PATH){
                /* cũ: set skin for path or portal */
                if( (Maze.Room[x][y].typ==ROOM_TYP_PATH) ||
                    (Maze.Room[x][y].special==ROOM_SPEC_PORTAL) )
                {
                    Maze.Room[x][y].skin= ROOM_SKIN_POINTS_NONE;
                    if(Maze.Room[x][y].points==ROOM_POINTS_NORMAL)
                        Maze.Room[x][y].skin= ROOM_SKIN_POINTS_NORMAL;
                    if(Maze.Room[x][y].points==ROOM_POINTS_ENERGY)
                        Maze.Room[x][y].skin= ROOM_SKIN_POINTS_ENERGY;
                }
            }
            else if(mark==SEARCH_SKIN_UNDEF){
                if(Maze.Room[x][y].skin==ROOM_SKIN_WALL_UNDEF){
                    Maze.Room[x][y].skin= room.skin;
                }
            }
        }
    }
}

/*----------------------------------------------------------------------
 * (Giữ nguyên) setportals, setgate
 *---------------------------------------------------------------------*/
uint32_t maze_generate_setportals(uint32_t x, uint32_t y, Room_t room)
{
    uint32_t ret_wert=0;
    if(x==0){
        if(y==0 || y>=(ROOM_CNT_Y-1)) return 1;
        /* portal A */
        Maze.Room[0][y].typ     = room.typ;
        Maze.Room[0][y].special = room.special;
        Maze.Room[0][y].door   |= ROOM_DOOR_L;
        /* portal B */
        Maze.Room[ROOM_CNT_X-1][y].typ= room.typ;
        Maze.Room[ROOM_CNT_X-1][y].special= room.special;
        Maze.Room[ROOM_CNT_X-1][y].door  |= ROOM_DOOR_R;
    }
    else if(x==(ROOM_CNT_X-1)){
        if(y==0 || y>=(ROOM_CNT_Y-1)) return 2;
        Maze.Room[ROOM_CNT_X-1][y].typ= room.typ;
        Maze.Room[ROOM_CNT_X-1][y].special= room.special;
        Maze.Room[ROOM_CNT_X-1][y].door |= ROOM_DOOR_R;
        Maze.Room[0][y].typ= room.typ;
        Maze.Room[0][y].special= room.special;
        Maze.Room[0][y].door |= ROOM_DOOR_L;
    }
    else if(y==0){
        if(x==0 || x>=(ROOM_CNT_X-1)) return 3;
        Maze.Room[x][0].typ= room.typ;
        Maze.Room[x][0].special= room.special;
        Maze.Room[x][0].door |= ROOM_DOOR_U;
        Maze.Room[x][ROOM_CNT_Y-1].typ= room.typ;
        Maze.Room[x][ROOM_CNT_Y-1].special= room.special;
        Maze.Room[x][ROOM_CNT_Y-1].door |= ROOM_DOOR_D;
    }
    else if(y==(ROOM_CNT_Y-1)){
        if(x==0 || x>=(ROOM_CNT_X-1)) return 4;
        Maze.Room[x][ROOM_CNT_Y-1].typ= room.typ;
        Maze.Room[x][ROOM_CNT_Y-1].special= room.special;
        Maze.Room[x][ROOM_CNT_Y-1].door |= ROOM_DOOR_D;
        Maze.Room[x][0].typ= room.typ;
        Maze.Room[x][0].special= room.special;
        Maze.Room[x][0].door |= ROOM_DOOR_U;
    }
    else{
        return 5;
    }
    return ret_wert;
}

uint32_t maze_generate_setgate(uint32_t x,uint32_t y, Room_t room, uint32_t door)
{
    uint32_t ret_wert=0;
    if(door==ROOM_DOOR_U){
        Maze.Room[x][y].special= room.special;
        Maze.Room[x][y].door   |= room.door;
        Maze.Room[x][y-1].door |= ROOM_DOOR_D;
        Maze.Room[x][y-1].typ   = ROOM_TYP_WALL;
    }
    else if(door==ROOM_DOOR_R){
        Maze.Room[x][y].special= room.special;
        Maze.Room[x][y].door   |= room.door;
        Maze.Room[x+1][y].door |= ROOM_DOOR_L;
        Maze.Room[x+1][y].typ   = ROOM_TYP_WALL;
    }
    else if(door==ROOM_DOOR_D){
        Maze.Room[x][y].special= room.special;
        Maze.Room[x][y].door   |= room.door;
        Maze.Room[x][y+1].door |= ROOM_DOOR_U;
        Maze.Room[x][y+1].typ   = ROOM_TYP_WALL;
    }
    else if(door==ROOM_DOOR_L){
        Maze.Room[x][y].special= room.special;
        Maze.Room[x][y].door   |= room.door;
        Maze.Room[x-1][y].door |= ROOM_DOOR_R;
        Maze.Room[x-1][y].typ   = ROOM_TYP_WALL;
    }
    else{
        ret_wert=1;
    }
    return ret_wert;
}

/*----------------------------------------------------------------------
 * 7) setwallskin_outside/inside => y nguyên cũ
 *---------------------------------------------------------------------*/
void maze_generate_setwallskin_outside(void)
{
    uint32_t x,y;
    for(x=0; x<ROOM_CNT_X; x++){
        if(Maze.Room[x][0].typ==ROOM_TYP_WALL && Maze.Room[x][0].skin==ROOM_SKIN_WALL_STD){
            maze_generate_setonewallskin_outside(x,0,0);
        }
        if(Maze.Room[x][ROOM_CNT_Y-1].typ==ROOM_TYP_WALL && Maze.Room[x][ROOM_CNT_Y-1].skin==ROOM_SKIN_WALL_STD){
            maze_generate_setonewallskin_outside(x, ROOM_CNT_Y-1,1);
        }
    }
    for(y=0; y<ROOM_CNT_Y; y++){
        if(Maze.Room[0][y].typ==ROOM_TYP_WALL && Maze.Room[0][y].skin==ROOM_SKIN_WALL_STD){
            maze_generate_setonewallskin_outside(0,y,2);
        }
        if(Maze.Room[ROOM_CNT_X-1][y].typ==ROOM_TYP_WALL && Maze.Room[ROOM_CNT_X-1][y].skin==ROOM_SKIN_WALL_STD){
            maze_generate_setonewallskin_outside(ROOM_CNT_X-1,y,3);
        }
    }
}

/* cũ => forward declare */
void maze_generate_setonewallskin_outside(uint32_t x, uint32_t y, uint32_t r);

void maze_generate_setonewallskin_outside(uint32_t x, uint32_t y, uint32_t r)
{
    /* default = UNDEF */
    Maze.Room[x][y].skin= ROOM_SKIN_WALL_UNDEF;
    /* try find the right skin */
    if(r==0){
        /* top */
        if(Maze.Room[x][y+1].typ==ROOM_TYP_PATH){
            Maze.Room[x][y].skin= ROOM_SKIN_WALL_O_TOP;
        }
    }
    else if(r==1){
        /* bottom */
        if(Maze.Room[x][y-1].typ==ROOM_TYP_PATH){
            Maze.Room[x][y].skin= ROOM_SKIN_WALL_O_BOTTOM;
        }
    }
    else if(r==2){
        /* left */
        if(Maze.Room[x+1][y].typ==ROOM_TYP_PATH){
            Maze.Room[x][y].skin= ROOM_SKIN_WALL_O_LEFT;
        }
    }
    else{
        /* right */
        if(Maze.Room[x-1][y].typ==ROOM_TYP_PATH){
            Maze.Room[x][y].skin= ROOM_SKIN_WALL_O_RIGHT;
        }
    }
}

void maze_generate_setwallskin_inside(void)
{
    uint32_t x,y;
    for(y=1; y<ROOM_CNT_Y-1; y++){
        for(x=1; x<ROOM_CNT_X-1; x++){
            if(Maze.Room[x][y].typ==ROOM_TYP_WALL && 
               Maze.Room[x][y].skin==ROOM_SKIN_WALL_STD)
            {
                maze_generate_setonewallskin_inside(x,y);
            }
        }
    }
}

void maze_generate_setonewallskin_inside(uint32_t x,uint32_t y)
{
    Maze.Room[x][y].skin= ROOM_SKIN_WALL_UNDEF; 
    /* cũ => set theo x,y-1 => PATH, etc. */
    if(Maze.Room[x][y-1].typ==ROOM_TYP_PATH){
        if(Maze.Room[x-1][y].typ==ROOM_TYP_PATH){
            Maze.Room[x][y].skin= ROOM_SKIN_WALL_I_TOPLEFT1;
        }
        else if(Maze.Room[x+1][y].typ==ROOM_TYP_PATH){
            Maze.Room[x][y].skin= ROOM_SKIN_WALL_I_TOPRIGHT1;
        }
        else{
            Maze.Room[x][y].skin= ROOM_SKIN_WALL_I_TOP;
        }
    }
    else if(Maze.Room[x][y+1].typ==ROOM_TYP_PATH){
        if(Maze.Room[x-1][y].typ==ROOM_TYP_PATH){
            Maze.Room[x][y].skin= ROOM_SKIN_WALL_I_BOTTOMLEFT1;
        }
        else if(Maze.Room[x+1][y].typ==ROOM_TYP_PATH){
            Maze.Room[x][y].skin= ROOM_SKIN_WALL_I_BOTTOMRIGHT1;
        }
        else{
            Maze.Room[x][y].skin= ROOM_SKIN_WALL_I_BOTTOM;
        }
    }
    else if(Maze.Room[x-1][y].typ==ROOM_TYP_PATH){
        Maze.Room[x][y].skin= ROOM_SKIN_WALL_I_LEFT;
    }
    else if(Maze.Room[x+1][y].typ==ROOM_TYP_PATH){
        Maze.Room[x][y].skin= ROOM_SKIN_WALL_I_RIGHT;
    }
    else if(Maze.Room[x-1][y-1].typ==ROOM_TYP_PATH){
        Maze.Room[x][y].skin= ROOM_SKIN_WALL_I_BOTTOMRIGHT1;
    }
    else if(Maze.Room[x-1][y+1].typ==ROOM_TYP_PATH){
        Maze.Room[x][y].skin= ROOM_SKIN_WALL_I_TOPRIGHT1;
    }
    else if(Maze.Room[x+1][y-1].typ==ROOM_TYP_PATH){
        Maze.Room[x][y].skin= ROOM_SKIN_WALL_I_BOTTOMLEFT1;
    }
    else if(Maze.Room[x+1][y+1].typ==ROOM_TYP_PATH){
        Maze.Room[x][y].skin= ROOM_SKIN_WALL_I_TOPLEFT1;
    }
}

/*----------------------------------------------------------------------
 * 8) setskin_h, setskin_v => y nguyên cũ
 *---------------------------------------------------------------------*/
uint32_t maze_generate_setskin_h(uint32_t x,uint32_t y,uint32_t l,uint32_t s)
{
    uint32_t ret=0;
    uint32_t n;
    if(x>=ROOM_CNT_X) return 1;
    if(y>=ROOM_CNT_Y) return 2;
    if(l<1) return 3;
    if((x+l)>ROOM_CNT_X) return 4;
    for(n=0; n<l; n++){
        Maze.Room[x+n][y].skin= s;
    }
    return ret;
}
uint32_t maze_generate_setskin_v(uint32_t x,uint32_t y,uint32_t l,uint32_t s)
{
    uint32_t ret=0;
    uint32_t n;
    if(x>=ROOM_CNT_X) return 1;
    if(y>=ROOM_CNT_Y) return 2;
    if(l<1) return 3;
    if((y+l)>ROOM_CNT_Y) return 4;
    for(n=0; n<l; n++){
        Maze.Room[x][y+n].skin= s;
    }
    return ret;
}

/*----------------------------------------------------------------------
 * 9) check, count_dots => y nguyên
 *---------------------------------------------------------------------*/
uint32_t maze_generate_check(void)
{
    uint32_t ret_wert=0;
    uint32_t x,y;
    /* 1) all rooms deb_err=OK */
    for(y=0; y<ROOM_CNT_Y; y++){
        for(x=0; x<ROOM_CNT_X; x++){
            Maze.Room[x][y].deb_err= ROOM_DEB_OK;
        }
    }
    /* 2) outline => no PATH */
    for(x=0; x<ROOM_CNT_X; x++){
        if(Maze.Room[x][0].typ==ROOM_TYP_PATH){
            ret_wert=1; 
            Maze.Room[x][0].deb_err |= ROOM_DEB_BORDER;
        }
        if(Maze.Room[x][ROOM_CNT_Y-1].typ==ROOM_TYP_PATH){
            ret_wert=2;
            Maze.Room[x][ROOM_CNT_Y-1].deb_err |= ROOM_DEB_BORDER;
        }
    }
    for(y=0; y<ROOM_CNT_Y; y++){
        if(Maze.Room[0][y].typ==ROOM_TYP_PATH){
            ret_wert=3; 
            Maze.Room[0][y].deb_err |= ROOM_DEB_BORDER;
        }
        if(Maze.Room[ROOM_CNT_X-1][y].typ==ROOM_TYP_PATH){
            ret_wert=4; 
            Maze.Room[ROOM_CNT_X-1][y].deb_err |= ROOM_DEB_BORDER;
        }
    }
    /* 3) all path has door */
    for(y=0; y<ROOM_CNT_Y; y++){
        for(x=0; x<ROOM_CNT_X; x++){
            if(Maze.Room[x][y].typ==ROOM_TYP_PATH){
                if(Maze.Room[x][y].door==ROOM_DOOR_NONE){
                    Maze.Room[x][y].deb_err |= ROOM_DEB_PATH;
                    ret_wert=5;
                }
            }
        }
    }
    /* 4) check doors has neighbor */
    {
        uint32_t xx,yy;
        for(yy=1; yy<ROOM_CNT_Y-1; yy++){
            for(xx=1; xx<ROOM_CNT_X-1; xx++){
                if(Maze.Room[xx][yy].typ==ROOM_TYP_PATH){
                    if(Maze.Room[xx][yy].door & ROOM_DOOR_U){
                        if((Maze.Room[xx][yy-1].door & ROOM_DOOR_D)==0){
                            Maze.Room[xx][yy].deb_err |= ROOM_DEB_DOOR;
                            ret_wert=6;
                        }
                    }
                    if(Maze.Room[xx][yy].door & ROOM_DOOR_D){
                        if((Maze.Room[xx][yy+1].door & ROOM_DOOR_U)==0){
                            Maze.Room[xx][yy].deb_err |= ROOM_DEB_DOOR;
                            ret_wert=7;
                        }
                    }
                    if(Maze.Room[xx][yy].door & ROOM_DOOR_L){
                        if((Maze.Room[xx-1][yy].door & ROOM_DOOR_R)==0){
                            Maze.Room[xx][yy].deb_err |= ROOM_DEB_DOOR;
                            ret_wert=8;
                        }
                    }
                    if(Maze.Room[xx][yy].door & ROOM_DOOR_R){
                        if((Maze.Room[xx+1][yy].door & ROOM_DOOR_L)==0){
                            Maze.Room[xx][yy].deb_err |= ROOM_DEB_DOOR;
                            ret_wert=9;
                        }
                    }
                }
            }
        }
    }
    /* 5) check all walls has skin */
    for(y=0; y<ROOM_CNT_Y; y++){
        for(x=0; x<ROOM_CNT_X; x++){
            if(Maze.Room[x][y].typ==ROOM_TYP_WALL){
                if(Maze.Room[x][y].skin==ROOM_SKIN_WALL_UNDEF){
                    Maze.Room[x][y].deb_err |= ROOM_DEB_SKIN;
                    ret_wert=10;
                }
            }
        }
    }
    /* 6) check dots>0 */
    {
        if(maze_generate_count_dots()==0){
            ret_wert=11;
        }
    }
    return ret_wert;
}

uint32_t maze_generate_count_dots(void)
{
    uint32_t ret=0;
    uint32_t x,y;
    for(y=0; y<ROOM_CNT_Y; y++){
        for(x=0; x<ROOM_CNT_X; x++){
            if(Maze.Room[x][y].points!=ROOM_POINTS_NONE){
                ret++;
            }
        }
    }
    return ret;
}

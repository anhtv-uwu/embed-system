/******************************************************************************
 * File: maze.c
 * Description:
 *   - Tạo maze ngẫu nhiên (auto) => Không còn các lệnh manually "setskin_h/v".
 *   - Gọi hàm random trong maze_generate.c => auto sinh PATH, WALL.
 *   - Sau đó, auto "set skin" (bên ngoài, bên trong) cho WALL,
 *     và "set skin path" cho đường đi.
 *   - Đếm dots.
 *
 * C89 style.
 *****************************************************************************/

#include "maze.h"
#include "random.h"
#include "stm32_ub_font.h"

#include "maze_generate.h"        /* Chứa maze_generate_init_random_map() */

/* Forward declarations */
void maze_make_rooms(void);
void maze_set_skin(void);
void maze_count_dots(void);

/*------------------------------------------------------------------------------
 * maze_build()
 *   - Hàm chính gọi để build maze:
 *     (1) Tạo map random,
 *     (2) Đặt skin auto,
 *     (3) Đếm dots.
 *----------------------------------------------------------------------------*/
void maze_build(void) 
{
    /* 1) Tạo (hoặc tái tạo) toàn bộ rooms => random */
    maze_make_rooms();

    /* 2) Đặt skin auto */
    maze_set_skin();

    /* 3) Đếm dots */
    maze_count_dots();
}

/*------------------------------------------------------------------------------
 * maze_make_rooms()
 *   - Thay cho code "fix cứng" cũ: 
 *   - Gọi hàm "maze_generate_init_random_map()" để random hoá map
 *----------------------------------------------------------------------------*/
void maze_make_rooms(void)
{
    /* Gọi hàm random (bạn đã cài trong maze_generate.c) */
    maze_generate_init_random_map();

    /* 
     * Nếu bạn muốn thêm logic “đặt portal/gate” hay “xóa 2 points ở (13,23)” 
     * thì thêm ở đây, 
     * nhưng hiện tại ta "auto hết" => Không manually.
     */
}

/*------------------------------------------------------------------------------
 * maze_set_skin()
 *   - Đặt skin “auto”:
 *     1) Tự động set skin cho path (SEARCH_SKIN_PATH).
 *     2) Auto set tường outside (maze_generate_setwallskin_outside).
 *     3) Auto set tường inside (maze_generate_setwallskin_inside).
 *     4) Cuối cùng, set WALL_UNDEF => BLACK (nếu còn sót).
 *----------------------------------------------------------------------------*/
void maze_set_skin(void)
{
    Room_t room;

    /* 1) set all "PATH" skins automatically */
    maze_generate_searchandset(SEARCH_SKIN_PATH, room);

    /* 2) set automatically the outside wall skins */
    maze_generate_setwallskin_outside();

    /* 3) set automatically the inside wall skins */
    maze_generate_setwallskin_inside();

    /* 4) set all "UNDEF" walls to "BLACK" */
    room.skin = ROOM_SKIN_WALL_BLACK;
    maze_generate_searchandset(SEARCH_SKIN_UNDEF, room);
}

/*------------------------------------------------------------------------------
 * maze_count_dots()
 *   - Gọi hàm “maze_generate_count_dots()” => lưu vào Maze.point_dots
 *----------------------------------------------------------------------------*/
void maze_count_dots(void)
{
    Maze.point_dots = maze_generate_count_dots();
}

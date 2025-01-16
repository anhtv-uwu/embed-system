//--------------------------------------------------------------
// Includes
//--------------------------------------------------------------
#include "pacman.h"

#include <stdio.h>

//--------------------------------------------------------------
// level settings
// speed in ms [Player, Blinky, Pinky, Inky, Clyde]
//--------------------------------------------------------------
Level_t Level[] = {
    {
        30,
        40,
        50,
        60,
        70
    },
    {
        26,
        35,
        44,
        52,
        60
    },
    {
        22,
        30,
        38,
        44,
        50
    },
    {
        18,
        25,
        32,
        36,
        40
    },
    {
        14,
        20,
        26,
        28,
        30
    },
    {
        10,
        18,
        24,
        26,
        28
    },
    {
        10,
        16,
        22,
        24,
        26
    },
    {
        10,
        14,
        20,
        22,
        24
    },
    {
        10,
        12,
        18,
        20,
        22
    },
    {
        10,
        10,
        16,
        18,
        20
    },
};

USB_HID_HOST_STATUS_t usb_status; // Trạng thái Keyboard
uint32_t pacman_hw_init(void);
uint32_t pacman_play(void);
void pacman_dec_mode_timer(void);
void pacman_init(uint32_t mode);
void pacman_set_level(void);

//--------------------------------------------------------------
// init and start endless pacman gameplay
//--------------------------------------------------------------
void pacman_start(void) {
    uint32_t check;
    char buf[10];

    Game.debug_mode = 0;
    Game.numberOfBots = 1;

    pacman_init(GAME_OVER);
    check = pacman_hw_init();

    if (check != 0) {
        UB_LCD_FillLayer(BACKGROUND_COL);
        UB_Font_DrawString(10, 280, "Touch ERR", & Arial_7x10, FONT_COL, BACKGROUND_COL);
        while (1);
    }

    // Khởi tạo Player và ghost
    player_init(GAME_OVER);
    blinky_init(GAME_OVER);
    pinky_init(GAME_OVER);
    inky_init(GAME_OVER);
    clyde_init(GAME_OVER);
    if (Game.mode_2_player == 1) {
        Blinky.move = MOVE_STOP;
    }
    // Gọi menu
    menu_start();

    skin_init();

    pacman_set_level();
    maze_build();
    check = maze_generate_check();

    if (check == 0) {
        // Maze OK
        while (1) {
            gui_draw_maze();
            gui_draw_bots();
            GUI.refresh_value = GUI_REFRESH_VALUE;
            GUI.refresh_buttons = GUI_REFRESH_VALUE;
            gui_draw_gui(GUI_JOY_NONE);

            UB_Font_DrawString(10, 305, "GET READY", & Arial_7x10, FONT_COL2, BACKGROUND_COL);
            UB_Systick_Pause_ms(1000);
            UB_Font_DrawString(10, 305, "    1    ", & Arial_7x10, FONT_COL2, BACKGROUND_COL);
            UB_Systick_Pause_ms(1000);
            UB_Font_DrawString(10, 305, "    2    ", & Arial_7x10, FONT_COL2, BACKGROUND_COL);
            UB_Systick_Pause_ms(1000);
            UB_Font_DrawString(10, 305, "    3    ", & Arial_7x10, FONT_COL2, BACKGROUND_COL);
            UB_Systick_Pause_ms(1000);
            UB_Font_DrawString(10, 305, "    GO   ", & Arial_7x10, FONT_COL2, BACKGROUND_COL);

            check = pacman_play();
            UB_Systick_Pause_ms(5000);

            if (check == GAME_PLAYER_WIN) {
                Player.level++;
            } else {
                Player.lives--;
                if (Player.lives == 0) {
                    check = GAME_OVER;
                }
            }

            // Chuẩn bị chơi lại
            pacman_init(check);
            player_init(check);
            blinky_init(check);
            pinky_init(check);
            inky_init(check);
            clyde_init(check);

            if (check == GAME_OVER) {
                // Quay lại menu
                menu_start();
                pacman_set_level();
            }
            if ((check == GAME_PLAYER_WIN) || (check == GAME_OVER)) {
                // Xây lại mê cung
                maze_build();
            }
        }
    } else {
        // Maze error
        gui_draw_errmaze();
        UB_Font_DrawString(10, 280, "MAZE ERR", & Arial_7x10, FONT_COL, BACKGROUND_COL);
        sprintf(buf, "last error nr = %d", (int)(check));
        UB_Font_DrawString(10, 290, buf, & Arial_7x10, FONT_COL, BACKGROUND_COL);
    }
}

//--------------------------------------------------------------
// init complete hardware
//--------------------------------------------------------------
uint32_t pacman_hw_init(void) {
    uint32_t ret_wert = 0;

    if (UB_Touch_Init() != SUCCESS) {
        ret_wert = 1;
    }

    UB_Systick_Init();
    UB_USB_HID_HOST_Init();
    UB_Uart_Init();
    UB_Button_Init();

    UB_LCD_Init();
    UB_LCD_LayerInit_Fullscreen();
    gui_clear_screen();

    return ret_wert;
}

//--------------------------------------------------------------
// init values
//--------------------------------------------------------------
void pacman_init(uint32_t mode) {
    if (mode == GAME_OVER) {
        Game.collision = BOOL_TRUE;
        Game.controller = GAME_CONTROL_4BUTTON;
    }
    Game.mode = GAME_MODE_SCATTER;
    Game.mode_timer = GAME_SCATTER_TIME;
    Game.frightened = BOOL_FALSE;
    Game.frightened_timer = GAME_FRIGHTENED_TIME;
    Game.frightened_points = GAME_FRIGHTENED_START_POINTS;
}

//--------------------------------------------------------------
// init Level
//--------------------------------------------------------------
void pacman_set_level(void) {
    if (Player.level <= GAME_MAX_LEVEL) {
        Player.akt_speed_ms = Level[Player.level - 1].player_speed;
        if (Game.mode_2_player ==1) {
            Blinky.akt_speed_ms = Level[Player.level - 1].player_speed;
            // Blinky.akt_speed_ms = -1;
            // Pinky.akt_speed_ms = -1;
            // Inky.akt_speed_ms = -1;
            // Clyde.akt_speed_ms = -1;
        }
        else {
            Blinky.akt_speed_ms = Level[Player.level - 1].blinky_speed;
            Pinky.akt_speed_ms = Level[Player.level - 1].pinky_speed;
            Inky.akt_speed_ms = Level[Player.level - 1].inky_speed;
            Clyde.akt_speed_ms = Level[Player.level - 1].clyde_speed;
        }
    } else {
        Player.akt_speed_ms = Level[GAME_MAX_LEVEL - 1].player_speed;
        if (Game.mode_2_player == 1) {
            Blinky.akt_speed_ms = Level[GAME_MAX_LEVEL - 1].player_speed;
            // Blinky.akt_speed_ms = -1;
            // Pinky.akt_speed_ms = -1;
            // Inky.akt_speed_ms = -1;  
            // Clyde.akt_speed_ms = -1;
        }
        else {
            Blinky.akt_speed_ms = Level[GAME_MAX_LEVEL - 1].blinky_speed;
            Pinky.akt_speed_ms = Level[GAME_MAX_LEVEL - 1].pinky_speed;
            Inky.akt_speed_ms = Level[GAME_MAX_LEVEL - 1].inky_speed;
            Clyde.akt_speed_ms = Level[GAME_MAX_LEVEL - 1].clyde_speed;
        }
    }
}

//--------------------------------------------------------------
// play one round pacman (until pacman die or level complete)
//--------------------------------------------------------------
uint32_t pacman_play(void) {
    uint32_t ret_wert = GAME_RUN;
    uint32_t joy = GUI_JOY_NONE;
    uint32_t joy_2_player = GUI_JOY_NONE;
    uint32_t movement = 0;
    int32_t pl_speed;

    // copy screen to background
    UB_LCD_Copy_Layer2_to_Layer1();

    do {
        //----------------------------------------
        // Touch/Button Timer
        //----------------------------------------
        if (Gui_Touch_Timer_ms == 0) {
            Gui_Touch_Timer_ms = GUI_TOUCH_INTERVALL_MS;

            joy = gui_check_button();
            // joy = gui_check_button_uart();
        }
        // joy = gui_check_button();

        movement = MOVE_NOBODY;

        //----------------------------------------
        // Player Timer
        //----------------------------------------
        if (Player_Systick_Timer_ms == 0) {
            if (Game.frightened == BOOL_FALSE) {
                Player_Systick_Timer_ms = Player.akt_speed_ms;
            } else {
                pl_speed = Player.akt_speed_ms - Player.frightened_buf;
                if (pl_speed < PLAYER_MAX_SPEED) pl_speed = PLAYER_MAX_SPEED;
                Player_Systick_Timer_ms = pl_speed;
            }
            movement |= MOVE_PLAYER;
        }
        //----------------------------------------
        // Blinky Timer
        //----------------------------------------
        if (Blinky_Systic_Timer_ms == 0) {
            if (Blinky.status == GHOST_STATUS_ALIVE) {
                if (Game.frightened == BOOL_FALSE) {
                    Blinky_Systic_Timer_ms = Blinky.akt_speed_ms;
                } else {
                    Blinky_Systic_Timer_ms = Blinky.akt_speed_ms + Blinky.frightened_buf;
                }
            } else {
                Blinky_Systic_Timer_ms = GHOST_DEAD_DELAY_MS;
            }
            movement |= MOVE_BLINKY;
        }

        //----------------------------------------
        // Pinky Timer
        //----------------------------------------
        if (Pinky_Systic_Timer_ms == 0) {
            if (Pinky.status == GHOST_STATUS_ALIVE) {
                if (Game.frightened == BOOL_FALSE) {
                    Pinky_Systic_Timer_ms = Pinky.akt_speed_ms;
                } else {
                    Pinky_Systic_Timer_ms = Pinky.akt_speed_ms + Pinky.frightened_buf;
                }
            } else {
                Pinky_Systic_Timer_ms = GHOST_DEAD_DELAY_MS;
            }
            movement |= MOVE_PINKY;
        }

        //----------------------------------------
        // Inky Timer
        //----------------------------------------
        if (Inky_Systic_Timer_ms == 0) {
            if (Inky.status == GHOST_STATUS_ALIVE) {
                if (Game.frightened == BOOL_FALSE) {
                    Inky_Systic_Timer_ms = Inky.akt_speed_ms;
                } else {
                    Inky_Systic_Timer_ms = Inky.akt_speed_ms + Inky.frightened_buf;
                }
            } else {
                Inky_Systic_Timer_ms = GHOST_DEAD_DELAY_MS;
            }
            movement |= MOVE_INKY;
        }

        //----------------------------------------
        // Clyde Timer
        //----------------------------------------
        if (Clyde_Systic_Timer_ms == 0) {
            if (Clyde.status == GHOST_STATUS_ALIVE) {
                if (Game.frightened == BOOL_FALSE) {
                    Clyde_Systic_Timer_ms = Clyde.akt_speed_ms;
                } else {
                    Clyde_Systic_Timer_ms = Clyde.akt_speed_ms + Clyde.frightened_buf;
                }
            } else {
                Clyde_Systic_Timer_ms = GHOST_DEAD_DELAY_MS;
            }
            movement |= MOVE_CLYDE;
        }

        //----------------------------------------
        // Mode Timer
        //----------------------------------------
        if (Mode_Systic_Timer_ms == 0) {
            Mode_Systic_Timer_ms = GAME_MODE_TIMER;
            pacman_dec_mode_timer();
        }

        //----------------------------------------
        // 1. clear all Bots
        //----------------------------------------
        if (movement != MOVE_NOBODY) {
            gui_clear_bots();
        }

        //----------------------------------------
        // 2a. Player movement
        //----------------------------------------
        if ((movement & MOVE_PLAYER) != 0) {
            player_move();
            player_change_direction(joy);
        }
        if (Game.mode_2_player == 1) {
            if ((movement & MOVE_BLINKY) != 0) {
                // joy_2_player = gui_check_button();
                joy_2_player = gui_check_button_uart();
                blinky_move_2_player_2(joy_2_player);
            }
        }
        else {  
            //----------------------------------------
            // 2b. Blinky movement
            //----------------------------------------
            if ((movement & MOVE_BLINKY) != 0) {
                blinky_move();
            }

            //----------------------------------------
            // 2c. Pinky movement
            //----------------------------------------
            if ((movement & MOVE_PINKY) != 0) {
                pinky_move();
            }

            //----------------------------------------
            // 2d. Inky movement
            //----------------------------------------
            if ((movement & MOVE_INKY) != 0) {
                inky_move();
            }

            //----------------------------------------
            // 2e. Clyde movement
            //----------------------------------------
            if ((movement & MOVE_CLYDE) != 0) {
                clyde_move();
            }
        }

        //----------------------------------------
        // 3. redraw all Bots + 4. LCD refresh
        //----------------------------------------
        if (movement != MOVE_NOBODY) {
            gui_draw_bots();
            gui_draw_gui(joy);
            UB_LCD_Refresh();
        }

        //----------------------------------------
        // check if game over
        //----------------------------------------
        if (Player.status == PLAYER_STATUS_DEAD) {
            ret_wert = GAME_PLAYER_LOSE;
        }
        if (Player.status == PLAYER_STATUS_WIN) {
            ret_wert = GAME_PLAYER_WIN;
        }
    }
    while (ret_wert == GAME_RUN);

    return ret_wert;
}

//--------------------------------------------------------------
// decrement mode timer and change mode if necessary
//--------------------------------------------------------------
void pacman_dec_mode_timer(void) {
    if (Game.frightened == BOOL_FALSE) {
        Game.mode_timer--;
        if (Game.mode_timer == 0) {
            GUI.refresh_value = GUI_REFRESH_VALUE;
            if (Game.mode == GAME_MODE_SCATTER) {
                Game.mode = GAME_MODE_CHASE;
                Game.mode_timer = GAME_CHASE_TIME;
            } else {
                Game.mode = GAME_MODE_SCATTER;
                Game.mode_timer = GAME_SCATTER_TIME;
            }
            if (Blinky.status == GHOST_STATUS_ALIVE) Blinky.new_mode = 1;
            if (Pinky.status == GHOST_STATUS_ALIVE) Pinky.new_mode = 1;
            if (Inky.status == GHOST_STATUS_ALIVE) Inky.new_mode = 1;
            if (Clyde.status == GHOST_STATUS_ALIVE) Clyde.new_mode = 1;
        }
    } else {
        Game.frightened_timer--;
        if (Game.frightened_timer == 0) {
            Game.frightened = BOOL_FALSE;
            Game.frightened_points = GAME_FRIGHTENED_START_POINTS;
        }
    }
}
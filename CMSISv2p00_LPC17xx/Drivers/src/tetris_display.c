#include "LPC17xx.h"
#include "tft_ili9341.h"
#include "tetris_display.h"
#include <stdio.h>

void Display_Init() {

	tft_init(TFT_ROT_180);
	tft_fill_screen(TFT_BLACK);
	Display_TetrisTitle();
}

void Display_Menu(enum StateMode state) {

	switch (state) {
		case DSP_MENU_SINGLE:
			Display_SelectSingleplayer();
			break;

		case DSP_MENU_MULTI:
			Display_SelectMultiplayer();
			break;

		case DSP_MENU_CREDITS:
			Display_SelectCredits();
			break;
		case DSP_CREDITS:
			Display_Credits();
			break;

		case DSP_WAITING_OPPONENT:
			Display_WaitingForPlayer();
			break;

		case DSP_SYNCING:
			Display_Syncing();
			break;

		case DSP_GAMEOVER_WON:
			Display_YouWon();
			break;

		case DSP_GAMEOVER_LOST:
			Display_YouLost();
			break;

		default:
			Display_SelectSingleplayer();
			break;
	}

}

void choose_Option(enum StateMode state) {

	switch (state) {
			case 0:
				// GOTO INICIO SINGLEPLAYER
				Display_RenderTetrisMap();
				break;

			case 1:
				// GOTO INICIO MULTIPLAYER
				Display_RenderTetrisMap();
				break;

			case 2:
				// GOTO INICIO CREDITS
				Display_Credits();
				break;

			default:
				// GOTO INICIO SINGLEPLAYER
				Display_RenderTetrisMap();
				break;
		}
}

void Display_TetrisTitle() {
	int16_t x = 35;
	int16_t y = 25;
	gfx_draw_char(x, y, 'T', TETRIS_ROJO, TFT_BLACK, 5);
	gfx_draw_char(x+=30, y, 'E', TETRIS_NARANJA, TFT_BLACK, 5);
	gfx_draw_char(x+=30, y, 'T', TETRIS_AMARILLO, TFT_BLACK, 5);
	gfx_draw_char(x+=30, y, 'R', TETRIS_VERDE, TFT_BLACK, 5);
	gfx_draw_char(x+=25, y, 'I', TETRIS_CYAN, TFT_BLACK, 5);
	gfx_draw_char(x+=25, y, 'S', TETRIS_VIOLETA, TFT_BLACK, 5);
}

void Display_ChangeSelectionMode(int16_t x, int16_t y, int16_t width, int16_t height) {
	gfx_fill_rect(x, y, width, height, TFT_BLACK);
}

void Display_SelectSingleplayer() {

	Display_TetrisTitle();
	Display_ChangeSelectionMode(0, 160, 150, 10);
	gfx_fill_rect(0, 105, 240, 24, TETRIS_SELECTION_MODE);
	gfx_draw_string(55, 110, "SINGLEPLAYER", TFT_WHITE, TETRIS_SELECTION_MODE, 2);
	gfx_draw_string(60, 160, "MULTIPLAYER", TFT_WHITE, TFT_BLACK, 2);
	gfx_draw_string(80, 238, "CREDITS", TFT_WHITE, TFT_BLACK, 2);
}

void Display_SelectMultiplayer() {

	Display_ChangeSelectionMode(0, 105, 150, 10);
	gfx_draw_string(55, 105, "SINGLEPLAYER", TFT_WHITE, TFT_BLACK, 2);
	gfx_fill_rect(0, 160, 240, 24, TETRIS_SELECTION_MODE);
	gfx_draw_string(60, 165, "MULTIPLAYER", TFT_WHITE, TETRIS_SELECTION_MODE, 2);
	Display_ChangeSelectionMode(0, 238, 150, 10);
	gfx_draw_string(80, 238, "CREDITS", TFT_WHITE, TFT_BLACK, 2);
}

void Display_SelectCredits() {

	Display_ChangeSelectionMode(0, 160, 150, 10);
	gfx_draw_string(55, 105, "SINGLEPLAYER", TFT_WHITE, TFT_BLACK, 2);
	gfx_draw_string(60, 160, "MULTIPLAYER", TFT_WHITE, TFT_BLACK, 2);
	gfx_fill_rect(0, 238, 240, 24, TETRIS_SELECTION_MODE);
	gfx_draw_string(80, 243, "CREDITS", TFT_WHITE, TETRIS_SELECTION_MODE, 2);
}

void Display_Credits() {

	Display_ChangeSelectionMode(0, 147, 150, 10);
	gfx_draw_string(8, 147, "Thanks for playing!!", TFT_WHITE, TFT_BLACK, 2);
	Display_ChangeSelectionMode(0, 260, 150, 10);
	gfx_draw_string(15, 260, "Made by: The Fantastic 4 @Copyright 2026", TFT_WHITE, TFT_BLACK, 1);
}

void Display_RenderTetrisMap() {

	tft_fill_screen(TFT_BLACK);
	gfx_draw_hline(5, 15, 10 * 16, TFT_WHITE);
	//gfx_draw_hline(5, 16, 10 * 16, TFT_WHITE);

	gfx_draw_hline(5, 16 * 19, 10 * 16, TFT_WHITE);
	//gfx_draw_hline(5, 16 * 19 + 1, 10 * 16, TFT_WHITE);

	gfx_draw_vline(5, 16, 18 * 16, TFT_WHITE);
	//gfx_draw_vline(5, 16, 18 * 16, TFT_WHITE);

	gfx_draw_vline(15 * 11 - 1, 16, 18 * 16, TFT_WHITE);
	//gfx_draw_vline(15 * 11, 16, 18 * 16, TFT_WHITE);
}

void Display_YouWon() {

	tft_fill_screen(TFT_BLACK);
	Display_TetrisTitle();
	// X viejo: 45, y viejo: 85
	//gfx_draw_circle(120, 160, 76,TETRIS_LOSERCOLOR);
	gfx_fill_circle(120, 160, 76, TETRIS_WINNER_COLOR);
	gfx_draw_string(35, 147, "YOU", TFT_WHITE, TFT_BLACK, 4);
	gfx_draw_string(30, 160, "WON!:D", TFT_WHITE, TFT_BLACK, 4);

	gfx_fill_star_5pct(75, 287, 50, TETRIS_STARS);
	gfx_fill_star_5pct(120, 287, 50, TETRIS_STARS);
	gfx_fill_star_5pct(163, 187, 50, TETRIS_STARS);
}

void Display_YouLost() {

	tft_fill_screen(TFT_BLACK);
	Display_TetrisTitle();
	// X viejo: 45, y viejo: 85
	//gfx_draw_circle(120, 160, 76,TETRIS_LOSERCOLOR);
	gfx_fill_circle(120, 160, 76, TETRIS_LOSER_COLOR);
	gfx_draw_string(35, 147, "YOU LOST! :(", TFT_WHITE, TFT_BLACK, 4);
	gfx_draw_string(30, 160, "LOST! :(", TFT_WHITE, TFT_BLACK, 4);

	gfx_draw_line(58, 279, 83, 303, TETRIS_LOSER_COLOR);
	gfx_draw_line(105, 279, 130, 303, TETRIS_LOSER_COLOR);
	gfx_draw_line(152, 279, 177, 303, TETRIS_LOSER_COLOR);
}

void Display_WaitingForPlayer() {

	tft_fill_screen(TFT_BLACK);
	Display_TetrisTitle();

	// mostramos los display cambiados
	gfx_fill_rect(0, 104, 240, 30, TFT_BLACK);
	switch (i) {

		case 0:
			gfx_draw_string(15, 147, "Waiting for player", TFT_WHITE, TFT_BLACK, 2);
			break;
		case 1:
			gfx_draw_string(15, 147, "Waiting for player", TFT_WHITE, TFT_BLACK, 2);
			gfx_draw_string(30, 150, ".", TFT_WHITE, TFT_BLACK, 2);
			break;
		case 2:
			gfx_draw_string(15, 147, "Waiting for player", TFT_WHITE, TFT_BLACK, 2);
			gfx_draw_string(29, 150, "..", TFT_WHITE, TFT_BLACK, 2);
			break;
		case 3:
			gfx_draw_string(15, 147, "Waiting for player", TFT_WHITE, TFT_BLACK, 2);
			gfx_draw_string(28, 150, "...", TFT_WHITE, TFT_BLACK, 2);
			break;

		default:
			i = 0;
			gfx_draw_string(5, 147, "Waiting for player", TFT_WHITE, TFT_BLACK, 2);
			break;
	}
	i = i + 1 % 4;
}

void Display_Syncing() {

	tft_fill_screen(TFT_BLACK);
	Display_TetrisTitle();
	// mostramos los display cambiando
	gfx_fill_rect(0, 104, 240, 30, TFT_BLACK);
	switch (i) {

		case 0:
			gfx_draw_string(15, 147, "Syncing", TFT_WHITE, TFT_BLACK, 3);
			break;
		case 1:
			gfx_draw_string(14, 147, "Syncing.", TFT_WHITE, TFT_BLACK, 3);
			break;
		case 2:
			gfx_draw_string(13, 147, "Syncing..", TFT_WHITE, TFT_BLACK, 3);
			break;
		case 3:
			gfx_draw_string(12, 147, "Syncing...", TFT_WHITE, TFT_BLACK, 3);
			break;

		default:
			gfx_draw_string(15, 147, "Syncing", TFT_WHITE, TFT_BLACK, 3);
			break;
	}
	i = i + 1 % 4;
}

void Display_UpdateActivePiece(uint8_t x, uint8_t y, uint32_t dataPieza) {
	gfx_draw_tetris_block(x, y, dataPieza);
}

void Display_UpdateHUD(int level, int score, uint32_t nextPiece) {
	char levelText[10];
	char scoreText[10];
	sprintf(levelText, "%d", level);
	sprintf(scoreText, "%d", score);

	// DIBUJAMOS EL TEXTO LEVEL Y SU NIVEL
	gfx_fill_rect(168, 21, 65, 178, TFT_BLACK);
	gfx_draw_string(LEVEL_TEXT_POSITION_X, LEVEL_TEXT_POSITION_Y, "Level:", TFT_WHITE, TFT_BLACK, 1);
	gfx_draw_string(LEVEL_NUM_POSITION_X, LEVEL_NUM_POSITION_Y, levelText, TFT_WHITE, TFT_BLACK, 1);
	// DIBUJAMOS EL TEXTO SCORE Y SU PUNTAJE
	gfx_draw_string(SCORE_TEXT_POSITION_X, SCORE_TEXT_POSITION_Y, "Score:", TFT_WHITE, TFT_BLACK, 1);
	gfx_draw_string(SCORE_POINTS_POSITION_X, SCORE_POINTS_POSITION_Y, scoreText, TFT_WHITE, TFT_BLACK, 1);
	// DIBUJAMOS EL TEXTO NEXT PIECE Y SU PIEZA
	gfx_draw_string(NEXT_PIECE_TEXT_POSITION_X, NEXT_PIECE_TEXT_POSITION_Y, "Next Piece:", TFT_WHITE, TFT_BLACK, 1);
	gfx_draw_mini_tetris_block(NEXT_PIECE_ICON_POSITION_X, NEXT_PIECE_ICON_POSITION_Y, nextPiece);
}

void Display_UpdateMap(MapaTetris *mapTetris) {
	int16_t x = 6;
	int16_t y = 17;
	for (int fila = 18; fila > 0; fila--) {
		y += TAM_BLOQUE_PX;
		if (mapTetris->mapa_matricial[fila][10] == 0) continue;
		for (int col = 0; col < 10; col++) {
			x += TAM_BLOQUE_PX;
			gfx_fill_tetris_block(x, y, mapTetris->mapa_matricial[fila][col]);
			gfx_draw_tetris_block(x, y, (mapTetris->mapa_matricial[fila][col] + 0x80000));
		}
		mapTetris->mapa_matricial[fila][10] = 0;
	}
};

#include "LPC17xx.h"
#include "tetris_display.h"
#include <stdio.h>

// VARIABLES QUIETAS DE LA UI
#define TEXT_SINGLEPLAYER_X0 55
#define TEXT_SINGLEPLAYER_Y0 110

#define TEXT_MULTIPLAYER_X0 60
#define TEXT_MULTIPLAYER_Y0 160

#define TEXT_CREDITS_X0 80
#define TEXT_CREDITS_Y0 238

#define TEXT_SHOWCREDITS_X0 8
#define TEXT_SHOWCREDITS_Y0 147

#define TEXT_TEAM_X0 5
#define TEXT_TEAM_Y0 260

#define TEXT_WINNER_X0 35
#define TEXT_WINNER_Y0 147

#define TEXT_LOOSER_X0 15
#define TEXT_LOOSER_Y0 147

#define TEXT_WAITING_X0 10
#define TEXT_WAITING_Y0 147

#define TEXT_SYNCING_X0 60
#define TEXT_SYNCING_Y0 147

#define BLACK_RECTANGLE_HEIGHT 30

// valor usado para la animacion de carga
static int i = -1;

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

	gfx_fill_rect(0, TEXT_SINGLEPLAYER_Y0-8, TFT_WIDTH, (BLACK_RECTANGLE_HEIGHT * 10), TFT_BLACK);

	gfx_fill_rect(0, TEXT_SINGLEPLAYER_Y0-8, TFT_WIDTH, BLACK_RECTANGLE_HEIGHT, TETRIS_SELECTION_MODE);
	gfx_draw_string(TEXT_SINGLEPLAYER_X0, TEXT_SINGLEPLAYER_Y0, "SINGLEPLAYER", TFT_WHITE, TETRIS_SELECTION_MODE, 2);

	gfx_fill_rect(0, TEXT_MULTIPLAYER_Y0-8, TFT_WIDTH, BLACK_RECTANGLE_HEIGHT, TFT_BLACK);
	gfx_draw_string(TEXT_MULTIPLAYER_X0, TEXT_MULTIPLAYER_Y0, "MULTIPLAYER", TFT_WHITE, TFT_BLACK, 2);

	gfx_draw_string(TEXT_CREDITS_X0, TEXT_CREDITS_Y0, "CREDITS", TFT_WHITE, TFT_BLACK, 2);
}

void Display_SelectMultiplayer() {

	Display_ChangeSelectionMode(0, TEXT_SINGLEPLAYER_Y0-8, TFT_WIDTH, BLACK_RECTANGLE_HEIGHT);
	gfx_draw_string(TEXT_SINGLEPLAYER_X0, TEXT_SINGLEPLAYER_Y0, "SINGLEPLAYER", TFT_WHITE, TFT_BLACK, 2);

	gfx_fill_rect(0, TEXT_MULTIPLAYER_Y0-8, TFT_WIDTH, BLACK_RECTANGLE_HEIGHT, TETRIS_SELECTION_MODE);
	gfx_draw_string(TEXT_MULTIPLAYER_X0, TEXT_MULTIPLAYER_Y0, "MULTIPLAYER", TFT_WHITE, TETRIS_SELECTION_MODE, 2);

	Display_ChangeSelectionMode(0, TEXT_CREDITS_Y0-8, TFT_WIDTH, BLACK_RECTANGLE_HEIGHT);
	gfx_draw_string(TEXT_CREDITS_X0, TEXT_CREDITS_Y0, "CREDITS", TFT_WHITE, TFT_BLACK, 2);
}

void Display_SelectCredits() {

	Display_ChangeSelectionMode(0, TEXT_MULTIPLAYER_Y0-8, TFT_WIDTH, BLACK_RECTANGLE_HEIGHT);
	gfx_draw_string(TEXT_MULTIPLAYER_X0, TEXT_MULTIPLAYER_Y0, "MULTIPLAYER", TFT_WHITE, TFT_BLACK, 2);

	gfx_fill_rect(0, TEXT_CREDITS_Y0-8, TFT_WIDTH, BLACK_RECTANGLE_HEIGHT, TETRIS_SELECTION_MODE);
	gfx_draw_string(TEXT_CREDITS_X0, TEXT_CREDITS_Y0, "CREDITS", TFT_WHITE, TETRIS_SELECTION_MODE, 2);
}

void Display_Credits() {

	gfx_fill_rect(0, TEXT_SINGLEPLAYER_Y0, TFT_WIDTH, (BLACK_RECTANGLE_HEIGHT * 10), TFT_BLACK);
	gfx_draw_string(TEXT_SHOWCREDITS_X0, TEXT_SHOWCREDITS_Y0, "Thanks for playing!!", TFT_WHITE, TFT_BLACK, 2);
	gfx_draw_string(TEXT_TEAM_X0, TEXT_TEAM_Y0, "Made by: The Fantastic 4 @Copyright 2026", TFT_WHITE, TFT_BLACK, 1);
}

void drawMapBorders(){
	gfx_draw_wire_rect(MAPA_X0-2, MAPA_Y0-1, MAPA_ANCHO +4, MAPA_ALTO +4, TFT_WHITE );
}

void Display_RenderTetrisMap() {

	tft_fill_screen(TFT_BLACK);
	drawMapBorders();
	//gfx_draw_hline(MAPA_X0-1, MAPA_Y0 -1, MAPA_ANCHO +2, TFT_WHITE);
	//gfx_draw_hline(5, 16, 10 * 16, TFT_WHITE);

	//gfx_draw_hline(MAPA_X0-1, MAPA_Y1 +1, MAPA_ANCHO +2, TFT_WHITE);
	//gfx_draw_hline(5, 16 * 19 + 1, 10 * 16, TFT_WHITE);

	//gfx_draw_vline(MAPA_X0-1, MAPA_Y0-1, MAPA_ALTO+2, TFT_WHITE);
	//gfx_draw_vline(5, 16, 18 * 16, TFT_WHITE);

	//gfx_draw_vline(MAPA_X1+1, MAPA_Y0-1, MAPA_ALTO+2, TFT_WHITE);
	//gfx_draw_vline(15 * 11, 16, 18 * 16, TFT_WHITE);
}

void Display_YouWon() {

	tft_fill_screen(TFT_BLACK);
	Display_TetrisTitle();
	// X viejo: 45, y viejo: 85
	//gfx_draw_circle(120, 160, 76,TETRIS_LOSERCOLOR);
	gfx_fill_circle(120, 160, 76, TETRIS_WINNER_COLOR);
	gfx_draw_string(TEXT_WINNER_X0, TEXT_WINNER_Y0, "YOU", TFT_WHITE, TFT_BLACK, 4);
	gfx_draw_string(TEXT_WINNER_X0-5, TEXT_WINNER_X0+13, "WON!:D", TFT_WHITE, TFT_BLACK, 4);

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
	gfx_draw_string(TEXT_LOOSER_X0, TEXT_LOOSER_Y0, "YOU LOST! :(", TFT_WHITE, TFT_BLACK, 4);
	gfx_draw_string(TEXT_LOOSER_X0-5, TEXT_LOOSER_Y0+13, "LOST! :(", TFT_WHITE, TFT_BLACK, 4);

	gfx_draw_line(58, 279, 83, 303, TETRIS_LOSER_COLOR);
	gfx_draw_line(105, 279, 130, 303, TETRIS_LOSER_COLOR);
	gfx_draw_line(152, 279, 177, 303, TETRIS_LOSER_COLOR);
}

void Display_WaitingForPlayer() {

	if (i == -1) {
		tft_fill_screen(TFT_BLACK);
		Display_TetrisTitle();
		i++;
	}

	// mostramos los display cambiados
	switch (i) {

		case 0:
			gfx_fill_rect(0, TEXT_WAITING_Y0+18, TFT_WIDTH-1, 30, TFT_BLACK);
			gfx_draw_string(TEXT_WAITING_X0, TEXT_WAITING_Y0, "Waiting for player", TFT_WHITE, TFT_BLACK, 2);
			break;
		case 1:
			gfx_draw_string(TEXT_WAITING_X0, TEXT_WAITING_Y0, "Waiting for player", TFT_WHITE, TFT_BLACK, 2);
			gfx_draw_string(TEXT_WAITING_X0*10, TEXT_WAITING_Y0+20, ".", TFT_WHITE, TFT_BLACK, 2);
			break;
		case 2:
			gfx_draw_string(TEXT_WAITING_X0, TEXT_WAITING_Y0, "Waiting for player", TFT_WHITE, TFT_BLACK, 2);
			gfx_draw_string(TEXT_WAITING_X0*10-1, TEXT_WAITING_Y0+20, "..", TFT_WHITE, TFT_BLACK, 2);
			break;
		case 3:
			gfx_draw_string(TEXT_WAITING_X0, TEXT_WAITING_Y0, "Waiting for player", TFT_WHITE, TFT_BLACK, 2);
			gfx_draw_string(TEXT_WAITING_X0*10-2, TEXT_WAITING_Y0+20, "...", TFT_WHITE, TFT_BLACK, 2);
			break;

		default:
			i = 0;
			gfx_fill_rect(0, TEXT_WAITING_Y0+18, TFT_WIDTH-1, 30, TFT_BLACK);
			break;
	}
	i = i + 1 % 4;
}

void Display_Syncing() {


	if (i != -1) {
		tft_fill_screen(TFT_BLACK);
		Display_TetrisTitle();
		i++;
	}

	switch (i) {

		case 0:
			gfx_draw_string(TEXT_SYNCING_X0, TEXT_SYNCING_Y0, "Syncing", TFT_WHITE, TFT_BLACK, 2);
			break;
		case 1:
			gfx_draw_string(TEXT_SYNCING_X0-1, TEXT_SYNCING_Y0, "Syncing.", TFT_WHITE, TFT_BLACK, 2);
			break;
		case 2:
			gfx_draw_string(TEXT_SYNCING_X0-2, TEXT_SYNCING_Y0, "Syncing..", TFT_WHITE, TFT_BLACK, 2);
			break;
		case 3:
			gfx_draw_string(TEXT_SYNCING_X0-3, TEXT_SYNCING_Y0, "Syncing...", TFT_WHITE, TFT_BLACK, 2);
			break;

		default:
			// mostramos los display cambiando
			gfx_fill_rect(TEXT_SYNCING_X0, 104, 240, 30, TFT_BLACK);
			break;
	}
	i = i + 1 % 4;
}

void Display_UpdateActivePiece(uint8_t x_old, uint8_t y_old, uint8_t x_new, uint8_t y_new, uint32_t dataPieza) {

	//if ((x < MAPA_X0 || (x +(16*4)) > MAPA_X1) && (y < MAPA_Y0 || (y + (16*4)) > MAPA_Y1)) return;
	if (x_old >= 0 &&  y_old >= 0) gfx_draw_tetris_block(x_old, y_old, (dataPieza & 0x0FFFF));
	gfx_draw_tetris_block(x_new, y_new, dataPieza);
	drawMapBorders();
}

void Display_PlacePiece(uint8_t x, uint8_t y, uint32_t dataPieza) {

	printf("LA PIEZA COLOCADA: %d\n ", dataPieza);
	gfx_fill_tetris_block(x, y, dataPieza);
	gfx_draw_tetris_block(x, y, (dataPieza + 0x80000));
	drawMapBorders();
}

void Display_UpdateHUD(int level, int score, uint32_t nextPiece) {
	char levelText[10];
	char scoreText[10];
	sprintf(levelText, "%d", level);
	sprintf(scoreText, "%d", score);

	// DIBUJAMOS EL TEXTO LEVEL Y SU NIVEL
	gfx_fill_rect(LEVEL_TEXT_POSITION_X, 0, 65, 300, TFT_BLACK);
	gfx_draw_string(LEVEL_TEXT_POSITION_X, LEVEL_TEXT_POSITION_Y, "Level:", TFT_WHITE, TFT_BLACK, 1);
	gfx_draw_string(LEVEL_NUM_POSITION_X, LEVEL_NUM_POSITION_Y, levelText, TFT_WHITE, TFT_BLACK, 1);
	// DIBUJAMOS EL TEXTO SCORE Y SU PUNTAJE
	gfx_draw_string(SCORE_TEXT_POSITION_X, SCORE_TEXT_POSITION_Y, "Score:", TFT_WHITE, TFT_BLACK, 1);
	gfx_draw_string(SCORE_POINTS_POSITION_X, SCORE_POINTS_POSITION_Y, scoreText, TFT_WHITE, TFT_BLACK, 1);
	// DIBUJAMOS EL TEXTO NEXT PIECE Y SU PIEZA
	gfx_draw_string(NEXT_PIECE_TEXT_POSITION_X, NEXT_PIECE_TEXT_POSITION_Y, "Next Piece:", TFT_WHITE, TFT_BLACK, 1);
	gfx_draw_mini_tetris_block(NEXT_PIECE_ICON_POSITION_X, NEXT_PIECE_ICON_POSITION_Y, nextPiece);
}

void Display_UpdateMap(uint8_t mapTetris[18][11]) {
	int16_t x = 0;
	int16_t y = 0;

	for (int fil = 17; fil > 0; fil--) {
		if (mapTetris[fil][10] == 0) continue;
		for (int col = 0; col < 10; col++) {

			int colorEnum = mapTetris[fil][col];
			uint16_t colorReal = gfx_get_color_by_enum(colorEnum);
			uint16_t colorSombra = gfx_get_color_by_enum((colorEnum) +8);
			y = MAPA_Y0 + (fil * TAM_BLOQUE_PX);
			x = MAPA_X0 + (col * TAM_BLOQUE_PX);
			if (colorEnum == 0) {gfx_fill_rect(x, y, TAM_BLOQUE_PX, TAM_BLOQUE_PX, colorReal);continue;}
			gfx_fill_rect(x, y, TAM_BLOQUE_PX, TAM_BLOQUE_PX, colorReal);
			gfx_draw_wire_rect(x, y, TAM_BLOQUE_PX, TAM_BLOQUE_PX, colorSombra);
		}
		mapTetris[fil][10] = 0;
	}
	drawMapBorders();
}

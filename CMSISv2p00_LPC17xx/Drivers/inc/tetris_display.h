#include "LPC17xx.h"
#include "tft_ili9341.h"
#include <stdio.h>

/** DEFINICIONES NECESARIAS **/
/* DEFINIMOS LOS LIMITES DEL MAPA POR PIXEL */
#define MAPA_ANCHO 160
#define MAPA_ALTO 288

#define MAPA_X0 5
#define MAPA_X1 165
#define MAPA_Y0 15
#define MAPA_Y1 304

/**  POSICIONES DE LA UI **/
//  LEVEL TEXT
#define LEVEL_TEXT_POSITION_X 190
#define LEVEL_TEXT_POSITION_Y 27
// LEVEL NUMBER
#define LEVEL_NUM_POSITION_X 201
#define LEVEL_NUM_POSITION_Y 44
// SCORE TEXT
#define SCORE_TEXT_POSITION_X 188
#define SCORE_TEXT_POSITION_Y 80
// SCORE NUMBER
#define SCORE_POINTS_POSITION_X 188
#define SCORE_POINTS_POSITION_Y 98
// NEXT PIECE TEXT
#define NEXT_PIECE_TEXT_POSITION_X 179
#define NEXT_PIECE_TEXT_POSITION_Y 152
// NEXT PIECE ICON
#define NEXT_PIECE_ICON_POSITION_X 199
#define NEXT_PIECE_ICON_POSITION_Y 170

// ----------------------------- INICIO DEL PROGRAMA ------------------------------------------------

enum StateMode { DSP_MENU_SINGLE, DSP_MENU_MULTI, DSP_MENU_CREDITS, DSP_CREDITS, DSP_WAITING_OPPONENT, DSP_SYNCING, DSP_GAMEOVER_WON, DSP_GAMEOVER_LOST };

/** @Brief Muestra el menu UI de un state
 * state: Pide el estado al que quiere ir
**/
void Display_Init();

/** @Brief Muestra el menu UI de un state
 * state: Pide el estado al que quiere ir
**/
void Display_Menu(enum StateMode state);

/** @Brief Muestra el menu UI de un state **/
void Display_TetrisTitle();

/** @Brief Cambia el menu de seleccion
 * int16_t x: posicion inicial en x del bloque de selección
 * int16_t y: posicion inicial en y del bloque de selección
 * int16_t witdh: ancho del bloque de selección
 * int16_t height: altura del bloque de selección
**/
void Display_ChangeSelectionMode(int16_t x, int16_t y, int16_t width, int16_t height);

/** @Brief Cambia el menu a seleccion SinglePlayer **/
void Display_SelectSingleplayer();

/** @Brief Cambia el menu a seleccion MultiPlayer **/
void Display_SelectMultiplayer();

/** @Brief Cambia el menu a seleccion de creditos **/
void Display_SelectCredits();

/** @Brief Cambia a los creditos **/
void Display_Credits();

/** @Brief Render inicial del mapa de tetris **/
void Display_RenderTetrisMap();

/** @Brief Display UI winner **/
void Display_YouWon();

/** @Brief Display UI loser **/
void Display_YouLost();

/** @Brief Display UI esperando jugador **/
void Display_WaitingForPlayer();

/** @Brief Display UI sincronizando jugador **/
void Display_Syncing();

/** @Brief Renderiza pieza activa
 * uint8_t x_old: Posicion inicial en x de la pieza a actualizar
 * uint8_t y_old: Posicion inicial en y de la pieza a actualizar
 * uint8_t x_new: Posicion inicial en x de la pieza a actualizar
 * uint8_t y_new: Posicion inicial en y de la pieza a actualizar
 * uint32_t dataPieza: 32 bits que contiene informacion de la pieza (color, forma y rotacion)
**/
void Display_UpdateActivePiece(uint8_t x_old, uint8_t y_old, uint8_t x_new, uint8_t y_new, uint32_t dataPieza);

/** @Brief Renderiza como "colocada" a la pieza activa cuando ya no puede moverse
 * uint8_t x: Posicion inicial en x de la pieza a actualizar
 * uint8_t y: Posicion inicial en y de la pieza a actualizar
 * uint32_t dataPieza: 32 bits que contiene informacion de la pieza (color, forma y rotacion)
**/
void Display_PlacePiece(uint8_t x, uint8_t y, uint32_t dataPieza);

/** @Brief Actualiza UI HUD
 * int level: Valor del nivel a actualizar
 * int score: Valor del score a actualizar
 * uint32_t y: 32 bits que contiene informacion de la pieza siguiente (color, forma y rotacion)
**/
void Display_UpdateHUD(int level, int score, uint32_t nextPiece);

/** @Brief Renderiza el mapa actualizado
 * mapTetris: Una matriz de matrices del mapa
**/
void Display_UpdateMap(uint8_t mapTetris[18][11]);

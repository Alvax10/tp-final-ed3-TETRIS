#ifdef __USE_CMSIS
#include "LPC17xx.h"
#include "lpc17xx_pinsel.h"
#include "lpc17xx_gpio.h"
#include "lpc17xx_timer.h"
#include "lpc17xx_adc.h"
#include "lpc17xx_gpdma.h"
#include "lpc17xx_uart.h"
#include "lpc17xx_i2s.h"
#include "tft_ili9341.h"
#include "tetris_display.h"
#include "perifericos.h"
#include "uart.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <cr_section_macros.h>


// Defines propios
#define TETRIS_WIDTH            11
#define TETRIS_HEIGHT           18

void Tetris_InitGame();
uint32_t Tetris_GeneratePiece(Bool next);
Bool Tetris_ValidateMove(uint32_t piece, int posX, int posY);
void Tetris_PlacePiece(uint32_t piece, int posX, int posY);
void Tetris_CheckLines();
void Tetris_CheckGameOver();
void Tetris_UpdateDifficulty(uint8_t level);
void Tetris_ReceiveAttack(uint8_t lines);

uint32_t mySeed;
uint32_t Tetris_InitRandom();
void Tetris_DelayMs(uint32_t ms);
//uint32_t Timer_GetTicks();

typedef enum {
    // Estados estáticos
    GAME_MENU = 1,
    GAME_CREDITS = 2,

    // Estados para el modo multijugador
    GAME_WAITING_OPPONENT = 3,
    GAME_SYNCING = 4,

    // Estado de la partida
    GAME_PLAYING = 5,
    GAME_OVER = 6
} GameState;

typedef enum {
    TETRIS_RIGHT         = 1,
    TETRIS_LEFT          = 2,
    TETRIS_UP            = 3,
    TETRIS_DOWN          = 4,
    TETRIS_SELECT        = 5
} InputAction;

typedef enum {
    COLOR_VACIO          = 0,
    COLOR_CYAN           = 1,
    COLOR_BLUE           = 2,
    COLOR_ORANGE         = 3,
    COLOR_YELLOW         = 4,
    COLOR_GREEN          = 5,
    COLOR_MAGENTA        = 6,
    COLOR_RED            = 7,
    COLOR_BLOCKED        = 8
} CellColor;

/*
typedef enum {
    MUSIC_MENU           = 0,
    MUSIC_WAITING        = 1,
    MUSIC_PLAYING        = 2,
    MUSIC_GAME_OVER      = 3,
    MUSIC_CREDITS        = 4,
    SFX_MOVE             = 5,
    SFX_SELECT           = 6,
    SFX_MOVE_PIECE       = 7,
    SFX_ROTATE           = 8,
    SFX_FALL             = 9,
    SFX_PLACE_PIECE      = 10,
    SFX_LINE_CLEAR       = 11,
    SFX_LINE_ATTACK      = 12
} SoundEvent;
*/
/*typedef enum {
    UA_NONE              = 0,
    UA_ROLE_MASTER       = 1,
    UA_ROLE_SLAVE        = 2,
    UA_SEED              = 3,
    UA_SEED_ACK          = 4,
    UA_SEND_ATTACK       = 5,
    UA_RECEIVE_ATTACK    = 6,
    UA_I_LOST            = 7
} UartEvent;*/

typedef struct {
    uint32_t last;
    uint32_t interval;
} Timer;

GameState currentState = GAME_MENU;
InputAction action = 0;
int currentFrame = 0;
UartEvent eventType = UA_NONE;


volatile uint32_t timerTicks = 0, lastDifficultyTimer = 0, intervalDifficultyTimer = 0;
Timer gravityTimer    = {0, 3};
Timer gameTimer       = {0, 1};
//Timer difficultyTimer = {0, 20000};
Timer creditsTimer    = {0, 5000};
Timer gameOverTimer   = {0, 5000};
Timer waitingTimer    = {0, 250};


Bool isMaster = false;
Bool seedSent = false;
Bool competitiveMode = false;

int x, y;

uint8_t newShape, currentShape, rotation = 0;
uint8_t linesToAdd = 0, linesToSend = 0, score = 0, level = 1;

uint16_t pieceOnly;
uint32_t currentPiece, nextPiece;
uint32_t nowTicks;

uint8_t board[TETRIS_HEIGHT][TETRIS_WIDTH] = {0};
const uint32_t SHAPES[7][4] = {
    // Pieza I
    {0x10F00, 0x14444, 0x10F00, 0x14444},
    // Pieza J
    {0x28E00, 0x26440, 0x20E20, 0x244C0},
    // Pieza L
    {0x32E00, 0x3C440, 0x30E80, 0x34460},
    // Pieza O
    {0x46600, 0x46600, 0x46600, 0x46600},
    // Pieza S
    {0x56C00, 0x54620, 0x506C0, 0x58C40},
    // Pieza T
    {0x64E00, 0x64640, 0x60E40, 0x64C40},
    // Pieza Z
    {0x7C600, 0x72640, 0x70C60, 0x74C80}
};

GameState (*StateHandler)(int input , int ticks);
GameState MenuState(int input , int ticks);
GameState CreditsState(int input , int ticks);
GameState WaitingOpponentState(int input , int ticks);
GameState SyncingState(int input , int ticks);
GameState PlayingState(int input , int ticks);
GameState GameOverState(int input , int ticks);

int SelectSinglePlayer(void);
int SelectMultiPlayer(void);
int SelectCredits(void);

const GameState (*State_To_Function[7]) (int, int)  = {
		[GAME_MENU] = MenuState,
		[GAME_CREDITS] = CreditsState,
		[GAME_WAITING_OPPONENT] = WaitingOpponentState,
		[GAME_SYNCING] = SyncingState,
		[GAME_PLAYING] = PlayingState,
		[GAME_OVER] = GameOverState
};

const int (*Selection_Opt[3]) (void) = {
		[DSP_MENU_SINGLE] = SelectSinglePlayer,
		[DSP_MENU_MULTI] = SelectMultiPlayer,
		[DSP_MENU_CREDITS] = SelectCredits
};



int main(void) {
    currentState = GAME_MENU;
    Peripheric_Init();
    Display_Init();
    Display_Menu(0);
    StateHandler = MenuState;
    //Sound_ChangeMusic(MUSIC_MENU);

    while(1){
    	int tick = Timer_GetTicks();
    	int inputs = Input_GetAction(TRUE);

    	int _newStateRequest = StateHandler(inputs, tick);

    	if (_newStateRequest != 0) {
    		currentState = _newStateRequest;
    		StateHandler = State_To_Function[_newStateRequest];
    	}
    }
}

void Tetris_InitGame() {
    for(uint8_t i = 0; i < TETRIS_HEIGHT; i++) {
        for(uint8_t j = 0; j < TETRIS_WIDTH; j++) {
            board[i][j] = 0;
        }
    }
    level = 1;
    score = 0;
    linesToAdd = 0;
    currentPiece = Tetris_GeneratePiece(FALSE);
    nextPiece = Tetris_GeneratePiece(TRUE);
    Display_RenderTetrisMap();
    Tetris_UpdateDifficulty(level);
    Display_UpdateHUD(level, score, nextPiece);
    //Sound_ChangeMusic(MUSIC_PLAYING);
    gameTimer.last = Timer_GetTicks();
}

uint32_t Tetris_GeneratePiece(Bool next) {
	int shape = rand() % 7;
	if (next){newShape = shape;}
	else{currentShape = shape;}
    rotation = 0;
    x = (TETRIS_WIDTH - 1) / 2 - 2;
    y = -2;
    return SHAPES[shape][rotation];
}

Bool Tetris_ValidateMove(uint32_t piece, int posX, int posY) {
    pieceOnly = (uint16_t)(piece & 0xFFFF);
    int boardX = 0;
    int boardY = 0;

    for(uint8_t i = 0; i < 4; i++) {
        for(uint8_t j = 0; j < 4; j++) {
            if((pieceOnly & (0x8000 >> (i * 4 + j))) != 0) { // Si la celda del bloque está ocupada
                boardX = posX + j;
                boardY = posY + i;
                if(boardY < 0) {
                    if(boardX < 0 || boardX >= (TETRIS_WIDTH - 1)) {return false;}
                    continue;   // Si está arriba, sólo validamos que no se salga de los costados
                }
                if(boardX < 0 || boardX >= (TETRIS_WIDTH - 1) || boardY >= TETRIS_HEIGHT) {
                    return false; // Fuera de los límites del tablero
                }
                if(board[boardY][boardX] != 0) {
                    return false; // Colisión con una pieza ya colocada
                }
            }
        }
    }
    return true;
}

void Tetris_PlacePiece(uint32_t piece, int posX, int posY) {
    uint8_t colorId = (uint8_t)(piece >> 16);
    for(uint8_t i = 0; i < 4; i++) {
        for(uint8_t j = 0; j < 4; j++) {

            if((piece & (0x8000 >> (i * 4 + j))) != 0) {

				int boardX = posX + j;
				int boardY = posY + i;

				if(boardX >= 0 && boardX < (TETRIS_WIDTH - 1) && boardY >= 0 && boardY < TETRIS_HEIGHT) {
					board[boardY][boardX] = colorId;	// Coloca la pieza en el tablero
					board[boardY][10] = 1; 				// fila cambio
					Display_PlacePiece(posX, posY, piece);
				}
            }
        }
    }

    //Sound_PlayEvent(SFX_PLACE_PIECE);
    currentPiece = nextPiece;
    currentShape = newShape;
    nextPiece = Tetris_GeneratePiece(TRUE);
    Display_UpdateHUD(level, score, nextPiece);

	//level++;
	//Tetris_UpdateDifficulty(level);
	//lastDifficultyTimer = Timer_GetTicks();
}

void Tetris_CheckLines() {
    linesToSend = 0;
    volatile Bool lineComplete = false;

    // Chequear lineas horizontales completas
    for(uint8_t row = 0; row < TETRIS_HEIGHT; row++) {
    	lineComplete = true;

        for(uint8_t col = 0; col < (TETRIS_WIDTH - 1); col++) {
            if(board[row][col] == 0) {
                lineComplete = false;
                break;
            }
        }

        if(lineComplete) {
            linesToSend++;
            for(uint8_t clearCol = 0; clearCol < (TETRIS_WIDTH - 1); clearCol++) {
                board[row][clearCol] = 0;
            }

            board[row][(TETRIS_WIDTH - 1)] = 1;
            Display_UpdateMap(board);
            //Sound_PlayEvent(SFX_LINE_CLEAR);
            Tetris_DelayMs(50);

            for(int8_t updRow = row; updRow >= 0; updRow--) {
                for(uint8_t updCol = 0; updCol < (TETRIS_WIDTH - 1); updCol++) {
                    if(updRow == 0) {
                        board[updRow][updCol] = COLOR_VACIO;
                    }
                    else {
                        board[updRow][updCol] = board[updRow - 1][updCol];
                    }
                }
                board[updRow][(TETRIS_WIDTH - 1)] = 1;
            }
            row--;
            Display_UpdateMap(board);
            //Sound_PlayEvent(SFX_PLACE_PIECE);
            Tetris_DelayMs(50);
        }
    }

    if(linesToSend > 0) {
        score += linesToSend * 100;
        if(score % 200 == 0){
        	level++;
        	Tetris_UpdateDifficulty(level);
        }
		Display_UpdateHUD(level, score, nextPiece);
        if(competitiveMode) {
            Uart_SendEvent(UA_SEND_ATTACK, linesToSend);
        }
    }
 }

void Tetris_CheckGameOver() {
    if(!Tetris_ValidateMove(currentPiece, x, y)) {
        currentState = GAME_OVER;
        Display_Menu(DSP_GAMEOVER_LOST);
        //Sound_ChangeMusic(MUSIC_GAME_OVER);
        gameOverTimer.last = Timer_GetTicks();
        if(competitiveMode) {
            Uart_SendEvent(UA_I_LOST, UA_NONE);
        }
    }
}

void Tetris_UpdateDifficulty(uint8_t level) {
    switch (1) {
        case 1: gravityTimer.interval = 3; break;
        case 2: gravityTimer.interval = 9; break;
        case 3: gravityTimer.interval = 8; break;
        case 4: gravityTimer.interval = 7; break;
        case 5: gravityTimer.interval = 6; break;
        case 6: gravityTimer.interval = 5; break;
        case 7: gravityTimer.interval = 4; break;
        default: gravityTimer.interval = 3; break;
    }
}


void Tetris_ReceiveAttack(uint8_t lines) {
    for(uint8_t i = 0; i < lines; i++) {
        // Mover todas las filas hacia arriba
        for(uint8_t row = 0; row < (TETRIS_HEIGHT - 1); row++) {
            for(uint8_t col = 0; col < (TETRIS_WIDTH - 1); col++) {
                board[row][col] = board[row + 1][col];
            }
            board[row][(TETRIS_WIDTH - 1)] = 1;
        }

        // Agregar una nueva fila con un bloque aleatorio
        for(uint8_t col = 0; col < (TETRIS_WIDTH - 1); col++) {
            board[TETRIS_HEIGHT - 1][col] = COLOR_BLOCKED;
        }
        board[TETRIS_HEIGHT - 1][rand() % (TETRIS_WIDTH - 1)] = 0;
        board[TETRIS_HEIGHT - 1][(TETRIS_WIDTH - 1)] = 1;
    }
    Uart_SendEvent(UA_RECEIVE_ATTACK, lines);
    Display_UpdateMap(board);
    //Sound_PlayEvent(SFX_LINE_ATTACK);
}

uint32_t Tetris_InitRandom() {
    uint16_t sec = LPC_RTC->SEC;
    uint16_t min = LPC_RTC->MIN;

    uint16_t seed = sec + (min * 60);
    return seed;
}

void Tetris_DelayMs(uint32_t ms) {
    for (volatile uint32_t i = 0; i < (ms * 12000); i++);
}


GameState MenuState(int input , int ticks){
	switch(input) {
		case TETRIS_UP: // Subir
			//Sound_PlayEvent(SFX_MOVE);
			if(currentFrame > DSP_MENU_SINGLE) currentFrame--;
			Display_Menu(currentFrame);
			break;
		case TETRIS_DOWN: // Bajar
			//Sound_PlayEvent(SFX_MOVE);
			if(currentFrame < DSP_MENU_CREDITS) currentFrame++;
			Display_Menu(currentFrame);
			break;
		case TETRIS_SELECT: // Seleccionar
			//Sound_PlayEvent(SFX_SELECT);
			return Selection_Opt[currentFrame]();
			break;
		default:
			break;
	}
	return 0;
}

GameState CreditsState(int input , int ticks){
	if(abs(ticks - creditsTimer.last) >= creditsTimer.interval) {
		currentFrame = 0;
		Display_Menu(currentFrame);
		//Sound_ChangeMusic(MUSIC_MENU);
		return GAME_MENU;
	}
	return 0;
}

GameState WaitingOpponentState(int input , int ticks){
	Display_Menu(DSP_WAITING_OPPONENT);

	/*if(Uart_IsConnected()){
		//eventType = Uart_CheckNetworkEvents();
		if(eventType == UA_ROLE_SLAVE) {
			isMaster = false;
		}
		currentState = GAME_SYNCING;
		Display_Menu(DSP_SYNCING);
	}
	else if(abs(nowTicks - waitingTimer.last) >= waitingTimer.interval) {
		Display_Menu(DSP_WAITING_OPPONENT);
		waitingTimer.last = Timer_GetTicks();
	}*/
	return 0;
}

GameState SyncingState(int input , int ticks) {

	if(isMaster) {
		if (!seedSent) {
			mySeed = Tetris_InitRandom();
			srand(mySeed);
			Uart_SendEvent(UA_SEED, mySeed);
			seedSent = true;
		}
		eventType = Uart_CheckNetworkEvents();
		if(eventType == UA_SEED_ACK) {
			seedSent = false;
			currentState = GAME_PLAYING;
			Tetris_InitGame();
		}
	}
	else {
		eventType = Uart_CheckNetworkEvents();
		if(eventType == UA_SEED) {
			mySeed = Uart_GetEventValue();
			srand(mySeed);
			Uart_SendEvent(UA_SEED_ACK, UA_NONE);
			currentState = GAME_PLAYING;
			Tetris_InitGame();
		}
	}
	return 0;
}

GameState GameOverState(int input , int ticks){
	if(abs(ticks - gameOverTimer.last) >= gameOverTimer.interval) {
		currentFrame = 0;
		Display_Menu(currentFrame);
		//Sound_ChangeMusic(MUSIC_MENU);
		return GAME_MENU;
	}
	return 0;
}

GameState PlayingState(int input , int ticks){

	if(abs(ticks - gameTimer.last) < gameTimer.interval) return 0;

	printf("GameTICK \n");

	switch(input) {
		case TETRIS_LEFT: // Mover izquierda
			if(Tetris_ValidateMove(currentPiece, x - 1, y)) {
				x--;
				printf("MUEVE IZQUIERDA \n");
				Display_UpdateActivePiece(x+1, y, x, y, currentPiece);
				//Sound_PlayEvent(SFX_MOVE_PIECE);
			}
		break;
		case TETRIS_RIGHT: // Mover derecha
			if(Tetris_ValidateMove(currentPiece, x + 1, y)) {
				x++;
				printf("MUEVE DERECHA \n");
				Display_UpdateActivePiece(x-1, y, x, y, currentPiece);
				//Sound_PlayEvent(SFX_MOVE_PIECE);
			}
		break;
		case TETRIS_UP: // Rotar
			if(Tetris_ValidateMove(SHAPES[currentShape][(rotation + 1) % 4], x, y)) {
				printf("ROTA PIEZA \n");
				rotation = (rotation + 1) % 4;
				Display_UpdateActivePiece(-1, y, x, y, (currentPiece & 0x0FFFF));
				currentPiece = SHAPES[currentShape][rotation];
				Display_UpdateActivePiece(-1, y, x, y, currentPiece);
				//Sound_PlayEvent(SFX_ROTATE);
			}
		break;
		case TETRIS_DOWN: // Soltar
			if(Tetris_ValidateMove(currentPiece, x, y + 1)) {
				printf("BAJA PIEZA \n");
				y++;
				Display_UpdateActivePiece(x, y-1, x, y, currentPiece);
				//Sound_PlayEvent(SFX_FALL);
			}
		break;

		default:
		break;
	}
	gameTimer.last = ticks;

	if(gravityTimer.last < gravityTimer.interval) { gravityTimer.last ++; return 0;}
	gravityTimer.last = 0;

	printf("Evento gravedad \n");

	if(Tetris_ValidateMove(currentPiece, x, y + 1)) {
		y++;
		Display_UpdateActivePiece(x, y-1, x, y, currentPiece);
		//Sound_PlayEvent(SFX_FALL);
	}
	else {
		Tetris_PlacePiece(currentPiece, x, y);
		Tetris_CheckLines();
		Tetris_CheckGameOver();
	}


	eventType = Uart_CheckNetworkEvents();
	if (eventType == UA_SEND_ATTACK) {
		linesToAdd = Uart_GetEventValue();
		Tetris_ReceiveAttack(linesToAdd);
	}
	else if(eventType == UA_I_LOST) {
		currentState = GAME_OVER;
		Display_Menu(DSP_GAMEOVER_WON);
		//Sound_ChangeMusic(MUSIC_GAME_OVER);
		gameOverTimer.last = Timer_GetTicks();
	}

	return 0;
}


int SelectSinglePlayer(void){
	mySeed = Tetris_InitRandom();
	srand(mySeed);
	Tetris_InitGame();
	return GAME_PLAYING;
}
int SelectMultiPlayer(void){
	competitiveMode = true;
	Display_Menu(DSP_WAITING_OPPONENT);
	//Sound_ChangeMusic(MUSIC_WAITING);
	waitingTimer.last = Timer_GetTicks();
	isMaster = true;
	Uart_Init(UA_ROLE_MASTER);
	return GAME_WAITING_OPPONENT;
}
int SelectCredits(void){
	Display_Menu(DSP_CREDITS);
	//Sound_ChangeMusic(MUSIC_CREDITS);
	creditsTimer.last = Timer_GetTicks();
	return GAME_CREDITS;
}

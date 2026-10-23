// Uso:  ./tetris           corre los tests y despues el juego
//       ./tetris --test    corre solo los tests (valgrind --leak-check=full ./tetris --test)

#include <SDL2/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// ========= CONSTANTES DEL JUEGO =========
#define SCREEN_WIDTH 500
#define SCREEN_HEIGHT 800
#define GRID_ROWS 20
#define GRID_COLS 10
#define CELL_SIZE 35
#define OFFSET_X 75
#define OFFSET_Y 50
#define TICK_RATE 30

const int TETROMINOS[7][4][4][4] = {
    { {{0,0,0,0},{1,1,1,1},{0,0,0,0},{0,0,0,0}}, {{0,0,1,0},{0,0,1,0},{0,0,1,0},{0,0,1,0}}, {{0,0,0,0},{0,0,0,0},{1,1,1,1},{0,0,0,0}}, {{0,1,0,0},{0,1,0,0},{0,1,0,0},{0,1,0,0}} },
    { {{2,0,0,0},{2,2,2,0},{0,0,0,0},{0,0,0,0}}, {{0,2,2,0},{0,2,0,0},{0,2,0,0},{0,0,0,0}}, {{0,0,0,0},{2,2,2,0},{0,0,2,0},{0,0,0,0}}, {{0,2,0,0},{0,2,0,0},{2,2,0,0},{0,0,0,0}} },
    { {{0,0,3,0},{3,3,3,0},{0,0,0,0},{0,0,0,0}}, {{0,3,0,0},{0,3,0,0},{0,3,3,0},{0,0,0,0}}, {{0,0,0,0},{3,3,3,0},{3,0,0,0},{0,0,0,0}}, {{3,3,0,0},{0,3,0,0},{0,3,0,0},{0,0,0,0}} },
    { {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}}, {{0,4,4,0},{0,4,4,0},{0,0,0,0},{0,0,0,0}} },
    { {{0,5,5,0},{5,5,0,0},{0,0,0,0},{0,0,0,0}}, {{0,5,0,0},{0,5,5,0},{0,0,5,0},{0,0,0,0}}, {{0,0,0,0},{0,5,5,0},{5,5,0,0},{0,0,0,0}}, {{5,0,0,0},{5,5,0,0},{0,5,0,0},{0,0,0,0}} },
    { {{0,6,0,0},{6,6,6,0},{0,0,0,0},{0,0,0,0}}, {{0,6,0,0},{0,6,6,0},{0,6,0,0},{0,0,0,0}}, {{0,0,0,0},{6,6,6,0},{0,6,0,0},{0,0,0,0}}, {{0,6,0,0},{6,6,0,0},{0,6,0,0},{0,0,0,0}} },
    { {{7,7,0,0},{0,7,7,0},{0,0,0,0},{0,0,0,0}}, {{0,0,7,0},{0,7,7,0},{0,7,0,0},{0,0,0,0}}, {{0,0,0,0},{7,7,0,0},{0,7,7,0},{0,0,0,0}}, {{0,7,0,0},{7,7,0,0},{7,0,0,0},{0,0,0,0}} }
};

SDL_Color colors[8] = {
    {30, 30, 30, 255}, {0, 255, 255, 255}, {0, 0, 255, 255}, {255, 165, 0, 255},
    {255, 255, 0, 255}, {0, 255, 0, 255}, {128, 0, 128, 255}, {255, 0, 0, 255}
};

// ========= ESTRUCTURAS DE DATOS =========

// 1. Nodo para la fila del tablero (Lista Enlazada)
typedef struct RowNode {
    int cells[GRID_COLS]; 
    struct RowNode* next; 
} RowNode;

// 2. Nodo para la cola de próximas piezas (Queue)
typedef struct PieceQueueNode {
    int type; 
    struct PieceQueueNode* next;
} PieceQueueNode;

// 3. Nodo para el sistema de partículas 
typedef struct ParticleNode {
    float x, y;
    float vel_x, vel_y;
    int color;
    int life_timer;
    struct ParticleNode* next;
} ParticleNode;

typedef struct ActivePiece {
    int col, row;
    int type;      
    int rotation;  
    int active;
} ActivePiece;

typedef struct GameBoard {
    RowNode* first_row;             
    PieceQueueNode* queue_head;     
    PieceQueueNode* queue_tail;     
    ParticleNode* first_particle;   
    ActivePiece current_piece;
    int score;
    int tick_timer;
} GameBoard;

// ========= VARIABLES GLOBALES =========
SDL_Window* window = NULL;
SDL_Renderer* renderer = NULL;
GameBoard* game_board = NULL;

// ========= EJERCICIO 1: STRINGS (NO USAR string.h) =========

char* strDuplicate(char* src) {
   int largo = 0;
   while (src[largo] != '\0') {
      largo++;
    }

    char* copia = malloc((largo + 1) * sizeof(char));

    if (copia == NULL) {
        return NULL;
    }

    for (int i = 0; i <= largo; i++) {
        copia[i] = src[i];
    }

    return copia;
}

int strCompare(char* s1, char* s2) {
    int i = 0;

    while (s1[i] == s2[i] && s1[i] != '\0') {
        i++;
    }

    if (s1[i] == s2[i]) {
        return 0;
    }

    if (s1[i] < s2[i]) {
        return 1;
    }

    return -1;
      
}

char* strConcatenate(char* src1, char* src2) {
    int largo1 = 0;
    int largo2 = 0;

    while (src1[largo1] != '\0') {
     largo1++;
    }

    while (src2[largo2] != '\0') {
     largo2++;
    }
    char* resultado = malloc((largo1 + largo2 + 1) * sizeof(char));
    if (resultado == NULL) {
      free(src1);
      free(src2);
    return NULL;
    }
    for (int i = 0; i < largo1; i++) {
        resultado[i] = src1[i];
    }
    for (int i = 0; i < largo2; i++) {
       resultado[largo1 + i] = src2[i];
    }

    resultado[largo1 + largo2] = '\0';
    free(src1);
    free(src2);

    return resultado;
}

// ========= EJERCICIO 2: FUNCIONES A IMPLEMENTAR (EL TP) =========

GameBoard* gameBoardNew() {

    GameBoard* board = malloc(sizeof(GameBoard));

    if (board == NULL) {
        return NULL;
    }

    board->first_row = NULL;
    board->queue_head = NULL;
    board->queue_tail = NULL;
    board->first_particle = NULL;

    board->current_piece.col = 0;
    board->current_piece.row = 0;
    board->current_piece.type = 0;
    board->current_piece.rotation = 0;
    board->current_piece.active = 0;

    board->score = 0;
    board->tick_timer = 0;

    RowNode* last_row = NULL;

    // Las filas se crean dinámicamente y se van conectando una detrás de otra
    for (int i = 0; i < GRID_ROWS; i++) {
        RowNode* new_row = malloc(sizeof(RowNode));

        if (new_row == NULL) {
            // Si falla un malloc, se libera todo lo que ya se había reservado
            RowNode* current = board->first_row;

            while (current != NULL) {
                RowNode* next = current->next;
                free(current);
                current = next;
            }

            free(board);
            return NULL;
        }

        for (int c = 0; c < GRID_COLS; c++) {
            new_row->cells[c] = 0;
        }

        new_row->next = NULL;

        // La primera fila se guarda en first_row; las siguientes se enlazan al final
        if (board->first_row == NULL) {
            board->first_row = new_row;
        } else {
            last_row->next = new_row;
        }

        last_row = new_row;
    }

    return board;
}
   
 

void gameBoardDelete(GameBoard* board) {

    if (board == NULL) {
        return;
    }

    RowNode* current_row = board->first_row;

    // Antes de liberar cada fila se guarda el next para poder seguir recorriendo la lista
    while (current_row != NULL) {
        RowNode* next_row = current_row->next;
        free(current_row);
        current_row = next_row;
    }

    PieceQueueNode* current_piece = board->queue_head;

    while (current_piece != NULL) {
        PieceQueueNode* next_piece = current_piece->next;
        free(current_piece);
        current_piece = next_piece;
    }

    ParticleNode* current_particle = board->first_particle;

    while (current_particle != NULL) {
        ParticleNode* next_particle = current_particle->next;
        free(current_particle);
        current_particle = next_particle;
    }
    // Una vez liberadas todas las estructuras internas, se libera el tablero
    free(board);
}



void gameBoardAddParticle(GameBoard* board, float x, float y, int color) {

    if (board == NULL) {
        return;
    }
     // Cada partícula ocupa su propio nodo reservado dinámicamente
    ParticleNode* new_particle = malloc(sizeof(ParticleNode));

    if (new_particle == NULL) {
        return;
    }

    new_particle->x = x;
    new_particle->y = y;

    new_particle->vel_x = ((rand() % 100) / 10.0f) - 5.0f;
    new_particle->vel_y = ((rand() % 50) / -10.0f) - 2.0f;

    new_particle->color = color;
    new_particle->life_timer = 30 + (rand() % 30);
    
    // La nueva partícula se agrega al principio de la lista
    new_particle->next = board->first_particle;
    board->first_particle = new_particle;
}

    


int gameBoardCheckAndClearLines(GameBoard* board) {
   
    if (board == NULL) {
        return 0;
    }

    int lines_cleared = 0;
    int row_index = 0;
    // Se usan current y previous para poder recorrer y desenlazar filas de la lista
    RowNode* current = board->first_row;
    RowNode* previous = NULL;

    while (current != NULL) {
        int is_full = 1;

        for (int c = 0; c < GRID_COLS; c++) {
            if (current->cells[c] <= 0) {
                is_full = 0;
                break;
            }
        }

        if (is_full) {
            RowNode* new_row = malloc(sizeof(RowNode));

            if (new_row == NULL) {
                return lines_cleared;
            }

            for (int c = 0; c < GRID_COLS; c++) {
                new_row->cells[c] = 0;
            }

            for (int c = 0; c < GRID_COLS; c++) {
                gameBoardAddParticle(
                    board,
                    OFFSET_X + c * CELL_SIZE + CELL_SIZE / 2,
                    OFFSET_Y + row_index * CELL_SIZE + CELL_SIZE / 2,
                    current->cells[c]
                );
            }

            RowNode* next = current->next;

            if (previous == NULL) {
                board->first_row = next;
            } else {
                previous->next = next;
            }
            // La fila completa ya fue desenlazada, así que ahora se puede liberar
            free(current);
           
            new_row->next = board->first_row;
            board->first_row = new_row;

            // Si se eliminó la primera fila, la nueva fila queda como previous
             
            if (previous == NULL) {
                previous = new_row;
            }

            current = next;
            lines_cleared++;
            row_index++;

        } else {
            previous = current;
            current = current->next;
            row_index++;
        }
    }

    return lines_cleared;
}

    
    

void gameBoardEnqueuePiece(GameBoard* board, int type) {
    
     if (board == NULL) {
        return;
    }

    // Se reserva un nodo nuevo para agregar una pieza a la cola
    PieceQueueNode* new_node = malloc(sizeof(PieceQueueNode));

    if (new_node == NULL) {
        return;
    }

    new_node->type = type;
    new_node->next = NULL;

    // Si la cola está vacía, el nuevo nodo es a la vez head y tail.
    if (board->queue_tail == NULL) {
        board->queue_head = new_node;
        board->queue_tail = new_node;
    } else {
        board->queue_tail->next = new_node;
        board->queue_tail = new_node;
    }
}
    
  
int gameBoardDequeuePiece(GameBoard* board, int* out_type) {
   if (board == NULL || out_type == NULL || board->queue_head == NULL) {
        return 0;
    }

    // Guardamos el nodo que va a salir antes de mover el head
    PieceQueueNode* node_to_remove = board->queue_head;

    *out_type = node_to_remove->type;

    board->queue_head = node_to_remove->next;

    // Si era el último nodo, la cola queda completamente vacía
    if (board->queue_head == NULL) {
        board->queue_tail = NULL;
    }

    free(node_to_remove);

    return 1;
}
   
   

// ========= LOGICA DE DIBUJADO Y COLISIONES (NO MODIFICAR) =========

RowNode* getRowAt(GameBoard* board, int index) {
    RowNode* curr = board->first_row;
    for(int i = 0; i < index && curr != NULL; i++) curr = curr->next;
    return curr;
}

int canMove(GameBoard* board, int target_col, int target_row, int target_rot) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            if (TETROMINOS[board->current_piece.type][target_rot][r][c] != 0) {
                int board_col = target_col + c;
                int board_row = target_row + r;
                if (board_col < 0 || board_col >= GRID_COLS || board_row >= GRID_ROWS) return 0;
                if (board_row >= 0) {
                    RowNode* row_node = getRowAt(board, board_row);
                    if (row_node && row_node->cells[board_col] != 0) return 0;
                }
            }
        }
    }
    return 1;
}

void lockPiece(GameBoard* board) {
    for (int r = 0; r < 4; r++) {
        for (int c = 0; c < 4; c++) {
            int val = TETROMINOS[board->current_piece.type][board->current_piece.rotation][r][c];
            if (val != 0) {
                RowNode* row_node = getRowAt(board, board->current_piece.row + r);
                if (row_node) row_node->cells[board->current_piece.col + c] = val;
            }
        }
    }
    board->current_piece.active = 0;
    board->score += gameBoardCheckAndClearLines(board) * 100;
}

// Devuelve 0 si la pieza nueva no entra (GAME OVER), 1 en otro caso.
int spawnNextPiece(GameBoard* board) {
    int type;
    if (gameBoardDequeuePiece(board, &type)) {
        board->current_piece.type = type;
        board->current_piece.rotation = 0;
        board->current_piece.col = GRID_COLS / 2 - 2;
        board->current_piece.row = 0;
        board->current_piece.active = 1;
        gameBoardEnqueuePiece(board, rand() % 7);
        if (!canMove(board, board->current_piece.col, board->current_piece.row, board->current_piece.rotation)) {
            printf("GAME OVER. Puntaje final: %d\n", board->score);
            board->current_piece.active = 0;
            return 0;   // main sale del loop y libera tablero y SDL
        }
    }
    return 1;
}

// Devuelve 0 cuando termina la partida.
int gameBoardUpdate(GameBoard* board) {
    if (!board) return 0;
    if (!board->current_piece.active && !spawnNextPiece(board)) return 0;

    board->tick_timer++;
    if (board->tick_timer >= TICK_RATE) {
        board->tick_timer = 0;
        if (canMove(board, board->current_piece.col, board->current_piece.row + 1, board->current_piece.rotation)) {
            board->current_piece.row++;
        } else {
            lockPiece(board);
        }
    }

    ParticleNode* curr_part = board->first_particle;
    ParticleNode* prev_part = NULL;
    while (curr_part) {
        curr_part->x += curr_part->vel_x;
        curr_part->y += curr_part->vel_y;
        curr_part->vel_y += 0.5f;
        curr_part->life_timer--;

        if (curr_part->life_timer <= 0) {
            ParticleNode* to_delete = curr_part;
            if (prev_part == NULL) {
                board->first_particle = curr_part->next;
                curr_part = board->first_particle;
            } else {
                prev_part->next = curr_part->next;
                curr_part = prev_part->next;
            }
            free(to_delete);
        } else {
            prev_part = curr_part;
            curr_part = curr_part->next;
        }
    }
    return 1;
}

void gameBoardDraw(GameBoard* board) {
    if (!board) return;
    SDL_SetRenderDrawColor(renderer, 30, 30, 30, 255);
    SDL_RenderClear(renderer);

    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_Rect board_rect = {OFFSET_X - 2, OFFSET_Y - 2, GRID_COLS * CELL_SIZE + 4, GRID_ROWS * CELL_SIZE + 4};
    SDL_RenderDrawRect(renderer, &board_rect);

    RowNode* current_row = board->first_row;
    int r = 0;
    while (current_row) {
        for (int c = 0; c < GRID_COLS; c++) {
            if (current_row->cells[c] > 0) {
                SDL_Color col = colors[current_row->cells[c]];
                SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
                SDL_Rect cell_rect = {OFFSET_X + c * CELL_SIZE, OFFSET_Y + r * CELL_SIZE, CELL_SIZE - 1, CELL_SIZE - 1};
                SDL_RenderFillRect(renderer, &cell_rect);
            }
        }
        current_row = current_row->next;
        r++;
    }

    if (board->current_piece.active) {
        for(int pr = 0; pr < 4; pr++) {
            for(int pc = 0; pc < 4; pc++) {
                int cell_val = TETROMINOS[board->current_piece.type][board->current_piece.rotation][pr][pc];
                if (cell_val != 0) {
                    SDL_Color col = colors[cell_val];
                    SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, 255);
                    SDL_Rect p_rect = {OFFSET_X + (board->current_piece.col + pc) * CELL_SIZE, 
                                       OFFSET_Y + (board->current_piece.row + pr) * CELL_SIZE, 
                                       CELL_SIZE - 1, CELL_SIZE - 1};
                    SDL_RenderFillRect(renderer, &p_rect);
                }
            }
        }
    }

    ParticleNode* curr_part = board->first_particle;
    while (curr_part) {
        SDL_Color col = colors[curr_part->color];
        int alpha = curr_part->life_timer * 8;   // se satura: SDL recibe un Uint8
        if (alpha > 255) alpha = 255;
        SDL_SetRenderDrawColor(renderer, col.r, col.g, col.b, alpha);
        SDL_Rect part_rect = {(int)curr_part->x, (int)curr_part->y, 6, 6};
        SDL_RenderFillRect(renderer, &part_rect);
        curr_part = curr_part->next;
    }
    SDL_RenderPresent(renderer);
}

int inicializar() {
    if (SDL_Init(SDL_INIT_VIDEO) < 0) return 0;
    window = SDL_CreateWindow("Tetris Base - TP", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, SCREEN_WIDTH, SCREEN_HEIGHT, SDL_WINDOW_SHOWN);
    if (window == NULL) return 0;
    renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (renderer == NULL) return 0;
    SDL_SetRenderDrawBlendMode(renderer, SDL_BLENDMODE_BLEND);
    return 1;
}

// ========= TESTS (COMPLETAR) =========

void correrTests() {
   // Casos minimos del enunciado:
   char* copia1 = strDuplicate("");
   char* copia2 = strDuplicate("x");
   char* copia3 = strDuplicate("hola");

   printf("Test strDuplicate 1 - Esperado: \"\" | Obtenido: \"%s\"\n", copia1);
   printf("Test strDuplicate 2 - Esperado: \"x\" | Obtenido: \"%s\"\n", copia2);
   printf("Test strDuplicate 3 - Esperado: \"hola\" | Obtenido: \"%s\"\n", copia3);

   free(copia1);
   free(copia2);
   free(copia3);

    

  printf("\nTests strCompare:\n");

  printf("Test 1 - \"\" vs \"\" | Esperado: 0 | Obtenido: %d\n",
       strCompare("", ""));

  printf("Test 2 - \"a\" vs \"a\" | Esperado: 0 | Obtenido: %d\n",
       strCompare("a", "a"));

  printf("Test 3a - \"a\" vs \"b\" | Esperado: 1 | Obtenido: %d\n",
       strCompare("a", "b"));

  printf("Test 3b - \"b\" vs \"a\" | Esperado: -1 | Obtenido: %d\n",
       strCompare("b", "a"));
    
  printf("\nTests strConcatenate:\n");

  char* src1 = strDuplicate("");
  char* src2 = strDuplicate("abc");
  char* concat1 = strConcatenate(src1, src2);

  printf("Test 1 - Esperado: \"abc\" | Obtenido: \"%s\"\n", concat1);

  free(concat1);


  char* src3 = strDuplicate("hola ");
  char* src4 = strDuplicate("mundo");
  char* concat2 = strConcatenate(src3, src4);

  printf("Test 2 - Esperado: \"hola mundo\" | Obtenido: \"%s\"\n", concat2);

  free(concat2);

  printf("\nTest Queue:\n");

GameBoard* board_queue = gameBoardNew();

if (board_queue == NULL) {
    printf("Test Queue - ERROR: no se pudo crear el tablero\n");
} else {
    gameBoardEnqueuePiece(board_queue, 1);
    gameBoardEnqueuePiece(board_queue, 2);
    gameBoardEnqueuePiece(board_queue, 3);
    gameBoardEnqueuePiece(board_queue, 4);
    gameBoardEnqueuePiece(board_queue, 5);

    int value;
    int queue_ok = 1;

    for (int expected = 1; expected <= 5; expected++) {
        int result = gameBoardDequeuePiece(board_queue, &value);

        if (result != 1 || value != expected) {
            queue_ok = 0;
        }
    }

    int sixth_result = gameBoardDequeuePiece(board_queue, &value);

    if (sixth_result != 0) {
        queue_ok = 0;
    }

    if (board_queue->queue_head != NULL ||
        board_queue->queue_tail != NULL) {
        queue_ok = 0;
    }

    gameBoardEnqueuePiece(board_queue, 6);

    int new_result = gameBoardDequeuePiece(board_queue, &value);

    if (new_result != 1 || value != 6) {
        queue_ok = 0;
    }

    printf("Test Queue - %s\n",
       queue_ok ? "CORRECTO" : "INCORRECTO");

    gameBoardDelete(board_queue);
  }
  GameBoard* board_row19 = gameBoardNew();

 if (board_row19 == NULL) {
    printf("Test fila 19 - INCORRECTO: no se pudo crear el tablero\n");
 } else {

    RowNode* row19 = getRowAt(board_row19, 19);

    for (int c = 0; c < GRID_COLS; c++) {
        row19->cells[c] = 1;
    }

    RowNode* old_first_row = board_row19->first_row;

    int cleared = gameBoardCheckAndClearLines(board_row19);

    int row_count = 0;
    RowNode* current_row = board_row19->first_row;

    while (current_row != NULL) {
        row_count++;
        current_row = current_row->next;
    }

    int first_row_empty = 1;

    for (int c = 0; c < GRID_COLS; c++) {
        if (board_row19->first_row->cells[c] != 0) {
            first_row_empty = 0;
        }
    }

    int row19_ok =
        cleared == 1 &&
        row_count == 20 &&
        board_row19->first_row != old_first_row &&
        first_row_empty;

    printf("Test fila 19 - %s\n",
           row19_ok ? "CORRECTO" : "INCORRECTO");

    gameBoardDelete(board_row19);
     }
    
printf("\nTest filas 0 y 1:\n");

GameBoard* board_rows01 = gameBoardNew();

if (board_rows01 == NULL) {
    printf("Test filas 0 y 1 - INCORRECTO: no se pudo crear el tablero\n");
} else {

    RowNode* row0 = getRowAt(board_rows01, 0);
    RowNode* row1 = getRowAt(board_rows01, 1);

    for (int c = 0; c < GRID_COLS; c++) {
        row0->cells[c] = 1;
        row1->cells[c] = 1;
    }

    int cleared = gameBoardCheckAndClearLines(board_rows01);

    int row_count = 0;
    RowNode* current_row = board_rows01->first_row;

    while (current_row != NULL) {
        row_count++;
        current_row = current_row->next;
    }

    int rows01_ok =
        cleared == 2 &&
        row_count == 20;

    printf("Test filas 0 y 1 - %s\n",
           rows01_ok ? "CORRECTO" : "INCORRECTO");

    gameBoardDelete(board_rows01);
     }
}



// Compara contra "--test" sin depender de string.h ni del Ejercicio 1.
int esFlagTest(char* arg) {
    char* flag = "--test";
    int i = 0;
    while (arg[i] != '\0' && arg[i] == flag[i]) i++;
    return arg[i] == '\0' && flag[i] == '\0';
}

int main(int argc, char* args[]) {
    srand(time(NULL));

    // Los tests corren antes de SDL. Con --test termina aca, asi valgrind solo
    // reporta la memoria del TP (SDL tiene bloques propios que no se liberan).
    correrTests();
    if (argc > 1 && esFlagTest(args[1])) return 0;

    if (!inicializar()) return 1;

    game_board = gameBoardNew();
    if (game_board == NULL) {
        printf("ADVERTENCIA: Implementa gameBoardNew() para iniciar el juego.\n");
        SDL_DestroyRenderer(renderer);
        SDL_DestroyWindow(window);
        SDL_Quit();
        return 0;
    }

    for(int i=0; i<3; i++) gameBoardEnqueuePiece(game_board, rand() % 7);

    SDL_Event e;
    int game_over = 0;

    while (!game_over) {
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) game_over = 1;
            if (e.type == SDL_KEYDOWN && game_board->current_piece.active) {
                if (e.key.keysym.sym == SDLK_LEFT && canMove(game_board, game_board->current_piece.col - 1, game_board->current_piece.row, game_board->current_piece.rotation)) game_board->current_piece.col--;
                if (e.key.keysym.sym == SDLK_RIGHT && canMove(game_board, game_board->current_piece.col + 1, game_board->current_piece.row, game_board->current_piece.rotation)) game_board->current_piece.col++;
                if (e.key.keysym.sym == SDLK_DOWN && canMove(game_board, game_board->current_piece.col, game_board->current_piece.row + 1, game_board->current_piece.rotation)) game_board->current_piece.row++;
                if (e.key.keysym.sym == SDLK_UP) {
                    int next_rot = (game_board->current_piece.rotation + 1) % 4;
                    if (canMove(game_board, game_board->current_piece.col, game_board->current_piece.row, next_rot)) game_board->current_piece.rotation = next_rot;
                }
                if (e.key.keysym.sym == SDLK_SPACE) {
                     while(canMove(game_board, game_board->current_piece.col, game_board->current_piece.row + 1, game_board->current_piece.rotation)) game_board->current_piece.row++;
                     lockPiece(game_board);
                }
            }
        }
        if (!gameBoardUpdate(game_board)) game_over = 1;
        gameBoardDraw(game_board);
        SDL_Delay(16);
    }
    gameBoardDelete(game_board);
    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();
    return 0;
}

#include <netdb.h>
#include <netinet/in.h>

#include <pthread.h>
#include <string.h>
#include <stdbool.h>
#include <wchar.h>
#include <locale.h>
#include <signal.h>
#include "database.c"
#include "board.c"
#define PORT 7000;
typedef struct roomdata {
  int roomID ;
  int player_is_waiting;
  int challenging_player;
  int state;
} roomdata;
// Waiting player conditional variable
pthread_cond_t player_to_join;
pthread_mutex_t general_mutex;
int challenging_player = 0;
int player_is_waiting = 0;
int count =0;
pthread_t tid[100];
user playersList[100000];
roomdata* roomList[100000];
int numOfPlayer;
int numOfRoom;
void* lobby(int playerID);
void* ReturnToLobby(void* id);
void init(roomdata* room,int player){
  room->state = 0;
  room->player_is_waiting = player;
  room->challenging_player = -1;
}
void join(roomdata* room,int player){
    if (room->state == -1) init(room,player);
    else {
      room->challenging_player = player;
      room->state = 1;
    }
  }
void clearroom(roomdata* room){
  room->state = -1;
  room->player_is_waiting = 0;
  room->challenging_player = 0; 
}
char* getInfor(roomdata * room){
  char* infor = (char*)malloc(sizeof(char) * 100); 
  char state[6];
  if (room->state == 0) strcpy(state,"ready");
  else strcpy(state,"full");
  sprintf(infor,"%d %s %s\n",room->roomID,rankNames[getRank(playersList[room->player_is_waiting].elo)],state);
  return infor;
}
char* getAllRoomInfor(){
  int i;
  char * allInfor = (char*) malloc(sizeof(char) * 1000);
  for (i=0;i<numOfRoom;++i){
    roomdata* room = roomList[i];
    if (room->state != -1){
      char* infor = getInfor(room);
      strcat(allInfor,infor);
      free(infor);
    }
  }
  return allInfor;
}

bool emit(int client, char *message, int message_size)
{
  return true;
}

void translate_to_move(int *move, char *buffer)
{
  *(move) = 8 - (*(buffer + 1) - '0');
  move[1] = (*(buffer) - '0') - 49;
  move[2] = 8 - (*(buffer + 4) - '0');
  move[3] = (*(buffer + 3) - '0') - 49;

  if (*(buffer + 5) == '-')
  {
    move[4] = *(buffer + 6) - '0';
  }

  printf("[%d, %d] to [%d, %d]\n", *(move), move[1], move[2], move[3]);
}

bool is_diagonal_clear(wchar_t **board, int *move)
{

  int *x_moves = (int *)malloc(sizeof(int));
  int *y_moves = (int *)malloc(sizeof(int));

  *(x_moves) = *(move)-move[2];
  *(y_moves) = move[1] - move[3];

  int *index = (int *)malloc(sizeof(int));
  *(index) = abs(*x_moves) - 1;
  wchar_t *item_at_position = (wchar_t *)malloc(sizeof(wchar_t));

  // Iterate thru all items excepting initial posi
  while (*index > 0)
  {

    if (*x_moves > 0 && *y_moves > 0)
    {
      printf("%lc [%d,%d]\n", board[*move - *index][move[1] - *index], *move - *index, move[1] - *index);
      *item_at_position = board[*move - *index][move[1] - *index];
    }
    if (*x_moves > 0 && *y_moves < 0)
    {
      printf("%lc [%d,%d]\n", board[*move - *index][move[1] + *index], *move - *index, move[1] + *index);
      *item_at_position = board[*move - *index][move[1] + *index];
    }
    if (*x_moves < 0 && *y_moves < 0)
    {
      printf("%lc [%d,%d]\n", board[*move + *index][move[1] + *index], *move + *index, move[1] + *index);
      *item_at_position = board[*move + *index][move[1] + *index];
    }
    if (*x_moves < 0 && *y_moves > 0)
    {
      printf("%lc [%d,%d]\n", board[*move + *index][move[1] - *index], *move + *index, move[1] - *index);
      *item_at_position = board[*move + *index][move[1] - *index];
    }

    if (*item_at_position != 0)
    {
      free(index);
      free(x_moves);
      free(y_moves);
      free(item_at_position);
      return false;
    }

    (*index)--;
  }

  free(index);
  free(x_moves);
  free(y_moves);
  free(item_at_position);

  return true;
}

bool is_syntax_valid(int player, char *buffer)
{
  if (strlen(buffer) < 5)
  {
    send(player, "e-11", 4, 0);
    return false;
  }

  // Look for -
  if (buffer[2] != '-')
  {
    send(player, "e-00", 4, 0);
    return false;
  }

  // First and 3th should be characters
  if (buffer[0] - '0' < 10)
  {
    send(player, "e-01", 4, 0);
    return false;
  }
  if (buffer[3] - '0' < 10)
  {
    send(player, "e-02", 4, 0);
    return false;
  }

  // Second and 5th character should be numbers
  if (buffer[1] - '0' > 10)
  {
    send(player, "e-03", 4, 0);
    return false;
  }
  if (buffer[1] - '0' > 8)
  {
    send(player, "e-04", 4, 0);
    return false;
  }
  if (buffer[4] - '0' > 10)
  {
    send(player, "e-05", 4, 0);
    return false;
  }
  if (buffer[4] - '0' > 8)
  {
    send(player, "e-06", 4, 0);
    return false;
  }
  // Move out of range
  if (buffer[0] - '0' > 56 || buffer[3] - '0' > 56)
  {
    send(player, "e-07", 4, 0);
    return false;
  }

  // Check when promoting pawn
  if (buffer[5] != 0)
  {
    if (buffer[5] != '-')
    {
      send(player, "e-08", 4, 0);
      return false;
    }
    if (buffer[6] - '0' > 10)
    {
      send(player, "e-09", 4, 0);
      return false;
    }
    if (buffer[6] - '0' < 1 || buffer[6] - '0' > 4)
    {
      send(player, "e-10", 4, 0);
      return false;
    }
  }

  return true;
}

void broadcast(wchar_t **board, char *one_dimension_board, int player_one, int player_two)
{

  to_one_dimension_char(board, one_dimension_board);

  printf("\tSending board to %d and %d size(%lu)\n", player_one, player_two, sizeof(one_dimension_board));
  send(player_one, one_dimension_board, 64, 0);
  send(player_two, one_dimension_board, 64, 0);
  printf("\tSent board...\n");
}

int get_piece_team(wchar_t **board, int x, int y)
{

  switch (board[x][y])
  {
  case white_king:
    return 1;
  case white_queen:
    return 1;
  case white_rook:
    return 1;
  case white_bishop:
    return 1;
  case white_knight:
    return 1;
  case white_pawn:
    return 1;
  case black_king:
    return -1;
  case black_queen:
    return -1;
  case black_rook:
    return -1;
  case black_bishop:
    return -1;
  case black_knight:
    return -1;
  case black_pawn:
    return -1;
  }

  return 0;
}

int get_piece_type(wchar_t piece)
{
  switch (piece)
  {
  case white_king:
    return 0;
  case white_queen:
    return 1;
  case white_rook:
    return 2;
  case white_bishop:
    return 3;
  case white_knight:
    return 4;
  case white_pawn:
    return 5;
  case black_king:
    return 0;
  case black_queen:
    return 1;
  case black_rook:
    return 2;
  case black_bishop:
    return 3;
  case black_knight:
    return 4;
  case black_pawn:
    return 5;
  }

  return -1;
}

void promote_piece(wchar_t **board, int destX, int destY, int target, int team)
{
  wchar_t piece;

  printf("Promoting piece -> %d\n", target);

  if (team == 1)
  {
    switch (target)
    {
    case 1:
      piece = white_queen;
      break;
    case 2:
      piece = white_bishop;
      break;
    case 3:
      piece = white_knight;
      break;
    case 4:
      piece = white_rook;
      break;
    default:
      piece = white_queen;
      break;
    }
  }
  else if (team == -1)
  {
    switch (target)
    {
    case 1:
      piece = black_queen;
      break;
    case 2:
      piece = black_bishop;
      break;
    case 3:
      piece = black_knight;
      break;
    case 4:
      piece = black_rook;
      break;
    default:
      piece = black_queen;
      break;
    }
  }

  board[destX][destY] = piece;
}

bool is_rect(int *move)
{

  int *x_moves = (int *)malloc(sizeof(int));
  int *y_moves = (int *)malloc(sizeof(int));

  *x_moves = *(move)-move[2];
  *y_moves = move[1] - move[3];

  if ((*x_moves != 0 && *y_moves == 0) || (*y_moves != 0 && *x_moves == 0))
  {
    free(x_moves);
    free(y_moves);
    return true;
  }

  free(x_moves);
  free(y_moves);
  return false;
}

int is_rect_clear(wchar_t **board, int *move, int x_moves, int y_moves)
{

  // Is a side rect
  int *index = (int *)malloc(sizeof(int));

  if (abs(x_moves) > abs(y_moves))
  {
    *index = abs(x_moves) - 1;
  }
  else
  {
    *index = abs(y_moves) - 1;
  }

  // Iterate thru all items excepting initial position
  while (*index > 0)
  {

    if (*(move)-move[2] > 0)
    {
      if (board[*move - (*index)][move[1]] != 0)
      {
        free(index);
        return false;
      }
    }
    if (*(move)-move[2] < 0)
    {
      if (board[*move + (*index)][move[1]] != 0)
      {
        free(index);
        return false;
      }
    }
    if (move[1] - move[3] > 0)
    {
      if (board[*move][move[1] - (*index)] != 0)
      {
        free(index);
        return false;
      }
    }
    if (move[1] - move[3] < 0)
    {
      if (board[*move][move[1] + (*index)] != 0)
      {
        free(index);
        return false;
      }
    }

    (*index)--;
  }

  free(index);
  return true;
}

bool is_diagonal(int x_moves, int y_moves)
{

  if ((abs(x_moves) - abs(y_moves)) != 0)
  {
    return false;
  }

  return true;
}

bool is_castling(wchar_t **board, int *move)
{
  int p1 = get_piece_type(board[*(move)][move[1]]);
  int p2 = get_piece_type(board[move[2]][move[3]]);
  int t1 = get_piece_team(board, *(move), move[1]);
  int t2 = get_piece_team(board, move[2], move[3]);

  if (t1 == t2)
  {
    if (p1 == 0 && p2 == 2 || p1 == 2 && p2 == 0)
    {
      return true;
    }
  }

  return false;
}

bool is_castling_clear(wchar_t **board, int row, int col1, int col2)
{
  int max = col1 > col2 ? col1 : col2;
  int min = col1 + col2 - max;

  for (int i = min + 1; i < max; i++)
  {
    if (board[row][i] != 0)
    {
      return false;
    }
  }

  return true;
}

int getManitud(int origin, int dest)
{
  return (abs(origin - dest));
}

bool eat_piece(wchar_t **board, int x, int y)
{
  return (get_piece_team(board, x, y) != 0);
}

void do_castle(wchar_t **board, int row, int col1, int col2)
{
  int min = col1 < col2 ? col1 : col2;
  int team = get_piece_team(board, row, col1);
  int y_moves = getManitud(col1, col2);

  wchar_t king = team == 1 ? white_king : black_king;
  wchar_t rook = team == 1 ? white_rook : black_rook;

  board[row][col1] = 0;
  board[row][col2] = 0;

  board[row][min + 2] = king;
  if (y_moves == 3)
  {
    board[row][min + 1] = rook;
  }
  else if (y_moves == 4)
  {
    board[row][min + 3] = rook;
  }
}

void move_piece(wchar_t **board, int *move)
{
  if (is_castling(board, move))
  {
    do_castle(board, *(move), move[1], move[3]);
    return;
  }

  // Move piece in board from origin to dest
  board[move[2]][move[3]] = board[*move][move[1]];
  board[*move][move[1]] = 0;
}

void freeAll(int *piece_team, int *x_moves, int *y_moves)
{
  free(piece_team);
  free(x_moves);
  free(y_moves);
}

bool is_move_valid(wchar_t **board, int player, int team, int *move, int *castle)
{

  int *piece_team = (int *)malloc(sizeof(int *));
  int *x_moves = (int *)malloc(sizeof(int *));
  int *y_moves = (int *)malloc(sizeof(int *));

  *piece_team = get_piece_team(board, *(move), move[1]);
  *x_moves = getManitud(*move, move[2]);
  *y_moves = getManitud(move[1], move[3]);

  // General errors
  if (board[*(move)][move[1]] == 0)
  {
    send(player, "m-00", 4, 0);
    return false;
  } // If selected piece == 0 there's nothing selected

  // Check if user is moving his piece
  if (team != *piece_team)
  {
    send(player, "m-01", 4, 0);
    return false;
  }

  // Check if it is castling
  if (is_castling(board, move))
  {
    // If player has castled or king/rook has moved
    if (*castle == 0)
    {
      send(player, "m-03", 4, 0);
      return false;
    }

    // If spaces between are not clear
    if (is_castling_clear(board, *(move), move[1], move[3]) == 0)
    {
      send(player, "m-04", 4, 0);
      return false;
    }

    *castle = 0;
    send(player, "i-97", 4, 0);
    return true;
  }

  if (*piece_team == get_piece_team(board, move[2], move[3]))
  {
    send(player, "m-02", 4, 0);
    return false;
  } // If the origin piece's team == dest piece's team is an invalid move

  printf("Player %d(%d) [%d,%d] to [%d,%d]\n", player, *piece_team, move[0], move[1], move[2], move[3]);

  // XMOVES = getManitud(*move, move[2])
  // YMOVES = getManitud(move[1], move[3])
  printf("Moved piece -> %d \n", get_piece_type(board[*(move)][move[1]]));
  switch (get_piece_type(board[*(move)][move[1]]))
  {
  case 0: /* --- ♚ --- */
    if (*x_moves > 1 || *y_moves > 1)
    {
      send(player, "m-10", 5, 0);
      freeAll(piece_team, x_moves, y_moves);
      return false;
    }

    if (*castle == 1)
    {
      *castle = 0;
    }

    if (eat_piece(board, move[2], move[3]))
    {
      send(player, "i-99", 4, 0);
      freeAll(piece_team, x_moves, y_moves);
      return true;
    }
    break;
  case 1: /* --- ♛ --- */
    if (!is_rect(move) && !is_diagonal(*x_moves, *y_moves))
    {
      send(player, "m-20", 5, 0);
      freeAll(piece_team, x_moves, y_moves);
      return false;
    }

    if (is_rect(move) && !is_rect_clear(board, move, *x_moves, *y_moves))
    {
      send(player, "m-21", 4, 0);
      freeAll(piece_team, x_moves, y_moves);
      return false;
    }

    if (is_diagonal(*x_moves, *y_moves) && !is_diagonal_clear(board, move))
    {
      send(player, "m-22", 4, 0);
      freeAll(piece_team, x_moves, y_moves);
      return false;
    }

    if (eat_piece(board, move[2], move[3]))
    {
      send(player, "i-99", 4, 0);
      freeAll(piece_team, x_moves, y_moves);
      return true;
    }
    break;
  case 2: /* --- ♜ --- */
    if (!is_rect(move))
    {
      send(player, "m-30", 5, 0);
      freeAll(piece_team, x_moves, y_moves);
      return false;
    }

    if (!is_rect_clear(board, move, *x_moves, *y_moves))
    {
      send(player, "m-31", 4, 0);
      freeAll(piece_team, x_moves, y_moves);
      return false;
    }

    if (*castle == 1)
    {
      *castle = 0;
    }

    if (eat_piece(board, move[2], move[3]))
    {
      send(player, "i-99", 4, 0);
      freeAll(piece_team, x_moves, y_moves);
      return true;
    }
    break;
  case 3: /* ––– ♝ ––– */
    if (!is_diagonal(*x_moves, *y_moves))
    {
      send(player, "m-40", 4, 0);
      freeAll(piece_team, x_moves, y_moves);
      return false;
    }
    if (!is_diagonal_clear(board, move))
    {
      send(player, "m-41", 4, 0);
      freeAll(piece_team, x_moves, y_moves);
      return false;
    }
    if (eat_piece(board, move[2], move[3]))
    {
      send(player, "i-99", 4, 0);
      freeAll(piece_team, x_moves, y_moves);
      return true;
    }
    break;
  case 4: /* --- ♞ --- */
    if ((abs(*x_moves) != 1 || abs(*y_moves) != 2) && (abs(*x_moves) != 2 || abs(*y_moves) != 1))
    {
      send(player, "m-50", 4, 0);
      freeAll(piece_team, x_moves, y_moves);
      return false;
    }
    if (eat_piece(board, move[2], move[3]))
    {
      send(player, "i-99", 4, 0);
      freeAll(piece_team, x_moves, y_moves);
      return true;
    }
    break;
  case 5: /* --- ♟ --- */
    if (*piece_team == 1 && move[2] == 0)
    {
      promote_piece(board, move[0], move[1], move[4], *piece_team);
      send(player, "i-98", 4, 0);
      freeAll(piece_team, x_moves, y_moves);
      return true;
    }
    if (*piece_team == -1 && move[2] == 7)
    {
      promote_piece(board, move[0], move[1], move[4], *piece_team);
      send(player, "i-98", 4, 0);
      freeAll(piece_team, x_moves, y_moves);
      return true;
    }

    if (*y_moves == 0)
    {
      // Check if it's the first move
      if (move[0] == 6 && *piece_team == 1 && *x_moves == 2)
      {
        printf("First move\n");
        return true;
      }
      if (move[0] == 1 && *piece_team == -1 && *x_moves == 2)
      {
        printf("First move\n");
        return true;
      }
      if (*x_moves > 1)
      {
        send(player, "m-60", 5, 0);
        freeAll(piece_team, x_moves, y_moves);
        return false;
      }
      if ((*move - move[2] == -1 && *piece_team == 1) || (*move - move[2] == 1 && *piece_team == -1))
      {
        send(player, "m-60", 5, 0);
        freeAll(piece_team, x_moves, y_moves);
        return false;
      }
      if (get_piece_team(board, move[2], move[3]) != 0)
      {
        send(player, "m-60", 5, 0);
        freeAll(piece_team, x_moves, y_moves);
        return false;
      }
    }
    else // Moving in Y axis
    {
      if (!is_diagonal(*x_moves, *y_moves) || (get_piece_team(board, move[2], move[3]) == 0))
      {
        send(player, "m-60", 4, 0);
        freeAll(piece_team, x_moves, y_moves);
        return false; // Check if it's a diagonal move and it's not an empty location
      }
      if (eat_piece(board, move[2], move[3]))
      {
        send(player, "i-99", 4, 0);
        freeAll(piece_team, x_moves, y_moves);
        return true; // Check if there's something to eat
      }
    }
    break;
  default:
    break;
  }

  freeAll(piece_team, x_moves, y_moves);
  return true;
}

bool is_game_over(wchar_t **board, int *winner)
{
  int white = 0, black = 0;

  for (int i = 0; i < 8; i++)
  {
    for (int j = 0; j < 8; j++)
    {
      if (board[i][j] == white_king)
      {
        white = 1;
      }
      if (board[i][j] == black_king)
      {
        black = 1;
      }
    }
  }

  if (white == 0)
  {
    *winner = -1;
    return true;
  }
  else if (black == 0)
  {
    *winner = 1;
    return true;
  }

  return false;
}


/* Calculate Elo After Finish GAme
   State = 0 : Draw 
   State > 0 : PLayer 1 Wins
   State < 0 : PLayer 2 Wins
*/
void calculateResult(int player1Id , int player2Id, int state){
  int eloDiff = playersList[player1Id].elo - playersList[player2Id].elo;
  int bonus = eloDiff/25;
  if (state < 0){ // Player 1 loses Player 2
    playersList[player2Id].elo = playersList[player2Id].elo + 16;
    playersList[player1Id].elo = playersList[player1Id].elo - 16;
    if (bonus > 0) {
      playersList[player2Id].elo += bonus;
      playersList[player1Id].elo -= bonus;
    }
  } else if (state == 0) { // Hoa
    playersList[player1Id].elo  -= bonus;
    playersList[player2Id].elo  += bonus;
  } else { // Player 1 Wins
    playersList[player1Id].elo += 16;
    playersList[player2Id].elo -= 16;
    if (bonus <0) {
      playersList[player1Id].elo -= bonus;
      playersList[player2Id].elo += bonus;
    }
  }
  changeElo(playersList[player1Id]);
  changeElo(playersList[player2Id]);
}

void * game_room(int roomID) {
  /* If connection is established then start communicating */
  roomdata* room = roomList[roomID];
  int player_one  = playersList[room->player_is_waiting].socket;//= *(int *)client_socket;
  int n, player_two;
  char buffer[64];
  int * move = (int *)malloc(sizeof(int)*4);
  // Create a new board
  wchar_t ** board = create_board();
  char * one_dimension_board = create_od_board();
  initialize_board(board);

  //player_is_waiting = 1; // Set user waiting

  //pthread_mutex_lock(&general_mutex); // Wait for player two
  //pthread_cond_wait(&player_to_join, &general_mutex); // Wait for player wants to join signal
  int *castle_1 = (int *)malloc(sizeof(int));
  *castle_1 = 1;
  int *castle_2 = (int *)malloc(sizeof(int));
  *castle_2 = 1;
  int *winner = (int *)malloc(sizeof(int));
  *winner = 0;
  // TODO lock assigning player mutex
  player_two = playersList[room->challenging_player].socket; // Asign the player_two to challenging_player
  printf("Room %d :\nPlayer One: %d\nPlayer Two: %d\n",roomID,player_one,room->challenging_player);
  player_is_waiting = 0; // Now none is waiting

  //pthread_mutex_unlock(&general_mutex); // Unecesary?
  if (send(player_one, "i-p1", 4, 0) < 0)
  {
    perror("ERROR writing to socket");
    exit(1);
  }
  if (send(player_two, "i-p2", 4, 0) < 0)
  {
    perror("ERROR writing to socket");
    exit(1);
  }

  sleep(1);

  // Broadcast the board to all the room players
  broadcast(board, one_dimension_board, player_one, player_two);

  sleep(1);

  bool syntax_valid = false;
  bool move_valid = false;

  while (1)
  {
    send(player_one, "i-tm", 4, 0);
    send(player_two, "i-nm", 4, 0);

    // Wait until syntax and move are valid
    printf("\nWaiting for move from player one (%d)\n", player_one);

    while (!syntax_valid || !move_valid)
    {
      bzero(buffer, 64);

      if ((n = read(player_one, buffer, 8)) < 0)
      {
        perror("ERROR reading from socket");
        exit(1);
      }
      buffer[n - 1] = 0;
      printf("Player one (%d) move: %s\n", player_one, buffer);

      syntax_valid = is_syntax_valid(player_one, buffer);

      if (syntax_valid)
      {
        translate_to_move(move, buffer); // Convert to move

        move_valid = is_move_valid(board, player_one, 1, move, castle_1);
      }
      else
      {
        move_valid = false;
      }

      printf("Checking syntax and move validation (%d,%d)\n", syntax_valid, move_valid);
    }

    printf("Player one (%d) made move\n", player_one);

    syntax_valid = false;
    move_valid = false;

    // Apply move to board
    move_piece(board, move);

    bzero(move, sizeof(move));

    // Send applied move board
    broadcast(board, one_dimension_board, player_one, player_two);
    sleep(1);

    // Check game is over or not
    if (is_game_over(board, winner))
    {
      break;
    }

    send(player_one, "i-nm", 4, 0);
    send(player_two, "i-tm", 4, 0);

    printf("\nWaiting for move from player two (%d)\n", player_two);

    while (!syntax_valid || !move_valid)
    {
      bzero(buffer, 64);

      if ((n = read(player_two, buffer, 8)) < 0)
      {
        perror("ERROR reading from socket");
        exit(1);
      }
      buffer[n - 1] = 0;
      printf("Player two (%d) move: %s\n", player_two, buffer);

      syntax_valid = is_syntax_valid(player_two, buffer);

      if (syntax_valid)
      {
        translate_to_move(move, buffer); // Convert to move

        move_valid = is_move_valid(board, player_two, -1, move, castle_2);
      }
      else
      {
        move_valid = false;
      }

      printf("Checking syntax and move validation (%d,%d)\n", syntax_valid, move_valid);
    }
    printf("Player two (%d) made move\n", player_two);

    syntax_valid = false;
    move_valid = false;

    // Apply move to board
    move_piece(board, move);

    bzero(move, sizeof(move));

    // Send applied move board
    broadcast(board, one_dimension_board, player_one, player_two);
    sleep(1);

    // Check game is over or not
    if (is_game_over(board, winner))
    {
      break;
    }
  }

  printf("\nWinner: %d\nLoser: %d\n\n", *winner, *winner * -1);
  if (*winner == 1)
  {
    send(player_one, "i-ow", 4, 0);
    send(player_two, "i-ol", 4, 0);
    calculateResult(room->player_is_waiting,room->challenging_player,1);
  }
  else
  {
    send(player_one, "i-ol", 4, 0);
    send(player_two, "i-ow", 4, 0);
    calculateResult(room->player_is_waiting,room->challenging_player,-1);
  }

  /* delete board and related things */
  free(move);
  free(castle_1);
  free(castle_2);
  free(winner);
  free_board(board);
  count ++;
  pthread_create(&tid[count], NULL, &ReturnToLobby, room);
  lobby(room->player_is_waiting);
}
void* ReturnToLobby(void* room){
  roomdata* dataRoom = (roomdata*) room;
  //printf("DON\' SEND DATA ERROR : %d\n",dataRoom->challenging_player);
  lobby(dataRoom->challenging_player);
}
int findNewRoom(){
  int i;
  for (i=0;i<numOfRoom;++i)
    if (roomList[i]->state == -1)
    return i;
  return numOfRoom;
}
void* lobby(int playerID){
  int player = playersList[playerID].socket;
  user data = playersList[playerID];
  printf("SEND DATA PLAYER %d TO SOCKET %d\n",playerID,player);
  send(player,convertDatatoString(data),2048,0);
  char buffer[2048];
  bzero(buffer,2048);
  buffer[0] = '\0';
  if (recv(player,buffer,1,0) <= 0){
    printf("[-] Disconnected %d\n",player);
    return;
  };
  printf("MESSAGE : %s\n",buffer);
  if (buffer[0] == '1'){
    send(player,getAllRoomInfor(),1000,0);
    recv(player,buffer,9,0);
    int roomID;
    sscanf(buffer,"%d",&roomID);
    roomdata* room = roomList[roomID];
    join(room,playerID);
    printf("Connected player %d, joining game room... %s\n",player,getInfor(room));
    game_room(roomID);
  } else if (buffer[0]=='2'){
    int emptyRoom = findNewRoom();
    free(roomList[emptyRoom]);
    roomdata* newroom = (roomdata*) malloc(sizeof(roomdata));
    clearroom(newroom);
    newroom->roomID = emptyRoom;
    join(newroom,playerID);
    roomList[emptyRoom] = newroom;
    if (emptyRoom+1>numOfRoom)
      numOfRoom = emptyRoom+1;
    sprintf(buffer,"%d",numOfRoom-1);
    printf("Connected player %d, creating new game room...\n",player);
    send(player,buffer,9,0);
  }
}
void* login(void* client_socket){
  int player = *(int *)client_socket;
  char buffer[2048];
  user data;
  bzero(buffer, 2048);
  int readfile;
  while(readfile = recv(player, buffer,2048,0)>0){
    if (readfile == -1){
      printf("Client Disconnect!");
      return client_socket;
    }
    data = convertStringtoData(buffer);
    if (data.elo == -1){
      data = getDataFromUserNameAndPassWord(data);
      if (data.elo == -1){
        send(player,"0",1,0);
      }
      else {
        send(player,"1",1,0);
        printf("Hello User!\n");
        break;
      }
    } else {
      int ok = addNewUser(data);
      if (ok) {
        send(player,"1",1,0);
        sleep(2);
        break;
      } else {
        send(player,"0",1,0);
      }
    }
  }
  //pthread_mutex_lock(&general_mutex); // Unecesary?
     // Create thread if we have no user waiting
  //printf("Player 1: %d",player_is_waiting);
  data.socket = player;
  //data.elo += 1000;
  //changeElo(data);
  int playerID = numOfPlayer;
  playersList[playerID]  = data;
  numOfPlayer++;
  lobby(playerID);
}
int main( int argc, char *argv[] ) {
  setlocale(LC_ALL, "en_US.UTF-8");
  readData();
  int sockfd, client_socket, port_number, client_length;
  char buffer[64];
  struct sockaddr_in server_address, client;
  int  n;

  // Conditional variable
  pthread_cond_init(&player_to_join, NULL);
  pthread_mutex_init(&general_mutex, NULL);

  /* First call to socket() function */
  sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd < 0) {
    perror("ERROR opening socket");
    exit(1);
  }

  /* Initialize socket structure */
  bzero((char *) &server_address, sizeof(server_address));
  port_number = PORT;

  server_address.sin_family = AF_INET;
  server_address.sin_addr.s_addr = INADDR_ANY;
  server_address.sin_port = htons(port_number);

  /* Now bind the host address using bind() call.*/
  if (bind(sockfd, (struct sockaddr *) &server_address, sizeof(server_address)) < 0) {
      perror("ERROR on binding");
      exit(1);
   }
  numOfPlayer = 0;
  numOfRoom   = 0;
          /* MAX_QUEUE */
  listen(sockfd, 20);
  printf("Server listening on port %d\n", port_number);
   while(1) {
     client_length = sizeof(client);
     // CHECK IF WE'VE A WAITING USER

     /* Accept actual connection from the client */
     client_socket = accept(sockfd, (struct sockaddr *)&client, (unsigned int *)&client_length);
     printf("– Connection accepted from %d at %d.%d.%d.%d:%d –\n", client_socket, client.sin_addr.s_addr&0xFF, (client.sin_addr.s_addr&0xFF00)>>8, (client.sin_addr.s_addr&0xFF0000)>>16, (client.sin_addr.s_addr&0xFF000000)>>24, client.sin_port);

     if (client_socket < 0) {
        perror("ERROR on accept");
        exit(1);
     }
    count ++;
    printf("ok\n");
    pthread_create(&tid[count], NULL, &login, &client_socket);// Unecesary?
     // If we've a user waiting join that room
   }

   close(sockfd);

   return 0;
}

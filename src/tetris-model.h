#pragma once
#include <stdbool.h>
#include <stdint.h>

typedef void*   tetris_hnd_t;
typedef uint8_t tetris_cell_t;
typedef struct
{
    int x, y;
} tetris_point_t;

// 테트리스 인스턴스를 만든다
tetris_hnd_t Tetris_Create();

// 테트리스 게임을 리셋한다
void Tetris_Reset( tetris_hnd_t );

// 현재 블록을 회전시킨다 ( 방향을 인자로 받음 )
// 회전 성공시 true, 실패하면 false 반환
bool Tetris_Turn( tetris_hnd_t, bool bClockWise );

// 현재 블록을 이동시킨다 ( 아래 좌 우 )
// Direction이 0이면 아래, -1이면 좌, 1이면 우
bool Tetris_Shift( tetris_hnd_t, int Direction );

// 현재 블록을 드랍한다
void Tetris_Drop( tetris_hnd_t );

// 업데이트 해준다 ( delta 시간을 전달 )
void Tetris_Update( tetris_hnd_t, float DeltaTime );

typedef struct
{
    tetris_cell_t const* Data;
    int                  Width;
    int                  Height;
    int                  score;
    tetris_point_t       CurBlockCenter;
    tetris_point_t       CurBlockShape[4];
} tetris_board_info_t;

// 현재 보드의 정보를 얻는다
tetris_board_info_t Tetris_GetBoardInfo( tetris_hnd_t );
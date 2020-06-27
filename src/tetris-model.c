#include "tetris-model.h"

#include <stdint.h>
#include <stdlib.h>
#include <string.h>

#define TETRIS_BOARD_WIDTH       10
#define TETRIS_BOARD_HEIGHT      20
#define TETRIS_DEFAULT_DROP_TIME 1.0f

enum BlockName
{
    Block_None = 0,
    Block_L,
    Block_RevL,
    Block_Sq,
    Block_I,
    Block_T,
    Block_Z,
    Block_S,
    Block_MAX = Block_S
};

static const tetris_point_t Shapes[][4] = {
  { { 0, 0 }, { 0, -2 }, { 0, -1 }, { 1, 0 } },
  { { 0, 0 }, { 0, -2 }, { 0, -1 }, { -1, 0 } },
  { { 0, 0 }, { 0, -1 }, { 1, -1 }, { 1, 0 } },
  { { 0, 0 }, { 0, -2 }, { 0, -1 }, { 0, 1 } },
  { { 0, 0 }, { -1, 0 }, { 0, -1 }, { 1, 0 } },
  { { 0, 0 }, { -1, -1 }, { 0, -1 }, { 1, 0 } },
  { { 0, 0 }, { 1, -1 }, { 0, -1 }, { -1, 0 } },
};

typedef struct tetris_implmentation
{
    // ?��?��리스�? 진행?��?�� 보드
    tetris_cell_t Board[TETRIS_BOARD_HEIGHT][TETRIS_BOARD_WIDTH];

    // ?��?�� ?��?���?�? ?��?�� 블록?�� ?���?
    // - ?���? ?���?
    tetris_point_t CurBlockCenter;
    // - 모양?�� ????�� ?���?
    tetris_point_t CurBlockShape[4];
    // - ?��?�� 블록
    int CurrentBlock;

    // ?��?�� 블록?�� ?��?���?기까�??�� ?���?
    float TimeLeftToAutoDrop;

    // 블록?�� ?��?���??�� 기�?? ?���?
    float AutoDropTime;
    
    int score;
} tetris_t;

/////////////////////////////////////////////////////////////////////////////
// ?���? ?��?�� ?��?��

// 블록?�� ?��?��?��
static void SpawnNewBlock( tetris_t* t );

// ?��?��?���? 블록?�� ?��?��?��
static void GenerateRandomBlock( tetris_point_t arr[] );

// 충돌 �??��
// 만약, �??��?��?��?�� ?��치에 충돌?�� 발생?��?�� 경우 true 반환
static bool CheckCollision(
  tetris_t*      t,
  tetris_point_t NextCenter,
  tetris_point_t NextShape[] );

// ?��?�� ?��?��
// 보드 ?��체�?? ?��캔해?�� ?��?���? ?�� 블록?�� 비우�? ?���? 보드�? shift
static void ScoreProcess( tetris_t* t );

// 굳히?�� ?��?��
// ?��?�� 블록?�� 보드?�� 고정?��?���? ?��로운 블록 ?��?��받음
static void HoldBlock( tetris_t* t );

/////////////////////////////////////////////////////////////////////////////
// ?��?�� ?��?��

// ?��?��리스 ?��?��?��?���? 만든?��
tetris_hnd_t Tetris_Create()
{
    tetris_t* inst = calloc( 1, sizeof( tetris_t ) );
    return inst;
}

// ?��?��리스 게임?�� 리셋?��?��
void Tetris_Reset( tetris_hnd_t h )
{
    tetris_t* t = h;
    memset(
      t->Board,
      0,
      sizeof( tetris_cell_t ) * TETRIS_BOARD_WIDTH * TETRIS_BOARD_HEIGHT );

    t->CurBlockCenter.x = TETRIS_BOARD_WIDTH / 2;
    SpawnNewBlock( t );
    t->AutoDropTime       = TETRIS_DEFAULT_DROP_TIME;
    t->TimeLeftToAutoDrop = TETRIS_DEFAULT_DROP_TIME;
}

// ?��?�� 블록?�� ?��?��?��?��?�� ( 방향?�� ?��?���? 받음 )
// ?��?�� ?��공시 true, ?��?��?���? false 반환
bool Tetris_Turn( tetris_hnd_t h, bool bClockWise )
{
    tetris_t*      t    = h;
    tetris_point_t next = t->CurBlockCenter;
    tetris_point_t *next_b = t->CurBlockShape;

    bool const QueryResult = CheckCollision( t, next, t->CurBlockShape );
    
    if ( QueryResult )
    {
        /* for (size_t i = 0; i < 4; i++)
        {
            if(t->CurBlockShape[i].x >= TETRIS_BOARD_WIDTH)
                t->CurBlockCenter.x -= 2;
                
            if(t->CurBlockShape[i].x < 0)
                t->CurBlockCenter.x += 2;
                
            if(t->CurBlockShape[i].y >= TETRIS_BOARD_HEIGHT)
                t->CurBlockCenter.y -= 2;
                
            if(t->CurBlockShape[i].y < 0)               
                t->CurBlockCenter.y += 2;
        } */
    }
    
    if ( bClockWise == 0 && QueryResult == false )
    {
        for (size_t i = 0; i < 4; i++)
        {
            int temp = next_b[i].x;
            t->CurBlockShape[i].x = next_b[i].y;     
            t->CurBlockShape[i].y = -temp; 
        }
    }
    
     if ( bClockWise == 1 && QueryResult == false )
    {
        for (size_t i = 0; i < 4; i++)
        {
            int temp = next_b[i].x;
            t->CurBlockShape[i].x = -next_b[i].y;     
            t->CurBlockShape[i].y = temp;  
        }
    }
    return !QueryResult;
}

// ?��?�� 블록?�� ?��?��?��?��?�� ( ?��?�� �? ?�� )
// Direction?�� 0?���? ?��?��, -1?���? �?, 1?���? ?��
bool Tetris_Shift( tetris_hnd_t h, int Direction )
{
    tetris_t*      t    = h;
    tetris_point_t next = t->CurBlockCenter;
    next.x += Direction;
    next.y += Direction == 0;

    bool const QueryResult = CheckCollision( t, next, t->CurBlockShape );

    if ( Direction == 0 && QueryResult )
    {
        HoldBlock( t );
        return false;
    }
    if ( Direction != 0 && QueryResult == false )
    {
        t->TimeLeftToAutoDrop
          = t->TimeLeftToAutoDrop < 0.3f ? 0.3f : t->TimeLeftToAutoDrop;
    }
    if ( QueryResult == false )
    {
        t->CurBlockCenter = next;
    }

    return !QueryResult;
}

// ?��?�� 블록?�� ?��?��?��?��
void Tetris_Drop( tetris_hnd_t h )
{
    while ( Tetris_Shift( h, 0 ) == true )
    {
    }
}

// ?��?��?��?�� ?���??�� ( delta ?��간을 ?��?�� )
void Tetris_Update( tetris_hnd_t h, float DeltaTime )
{
    tetris_t* t = h;

    t->TimeLeftToAutoDrop -= DeltaTime;
    if ( t->TimeLeftToAutoDrop < 0 )
    {
        t->TimeLeftToAutoDrop = t->AutoDropTime;
        Tetris_Shift( t, 0 );
    }
}

tetris_board_info_t Tetris_GetBoardInfo( tetris_hnd_t h )
{
    tetris_t*           t = h;
    tetris_board_info_t r;
    r.Data           = t->Board;
    r.CurBlockCenter = t->CurBlockCenter;
    memcpy( r.CurBlockShape, t->CurBlockShape, sizeof( t->CurBlockShape ) );
    r.Height = TETRIS_BOARD_HEIGHT;
    r.Width  = TETRIS_BOARD_WIDTH;
    r.score = t->score;
    return r;
}

static void HoldBlock( tetris_t* t )
{
    for ( size_t i = 0; i < 4; i++ )
    {
        int x = t->CurBlockCenter.x + t->CurBlockShape[i].x;
        int y = t->CurBlockCenter.y + t->CurBlockShape[i].y;

        t->Board[y][x] = t->CurrentBlock;
    }

    SpawnNewBlock( t );
    ScoreProcess( t );
}

static void ScoreProcess( tetris_t* t )
{
    int j = 0;
    int mul = 1;
    int acq[4];
    for (size_t i = 0; i < TETRIS_BOARD_HEIGHT; i++)
    {
        // �� �� �� ���غ�
        for (size_t k = 0; k < TETRIS_BOARD_WIDTH; k++)
        {
            mul = mul * t->Board[i][k];
        }
        // ���� �� ���� ��ü ���� 0�̸� ��ĭ�� �ִ� ��
        // �׷��� ������ ����!
        if(mul != 0)
        {
            t->score += 1;
            acq[j] = i;
            for (size_t z = 0; z < TETRIS_BOARD_WIDTH; z++)
                {
                    t->Board[i][z] = 0;
                }
            j++;
        }
        mul = 1;
    }
    
}

static void SpawnNewBlock( tetris_t* t )
{
    // ** Temporary **
    t->CurBlockCenter.y = 2;
    t->CurBlockCenter.x = TETRIS_BOARD_WIDTH / 2;
    t->CurrentBlock     = ( rand() % Block_MAX ) + 1;

    tetris_point_t const* Shape = Shapes[t->CurrentBlock - 1];
    memcpy( t->CurBlockShape, Shape, sizeof( t->CurBlockShape ) ); 
}

static bool CheckCollision(
  tetris_t*      t,
  tetris_point_t NextCenter,
  tetris_point_t NextShape[] )
{
    for ( size_t i = 0; i < 4; i++ )
    {
        int x = NextCenter.x + NextShape[i].x;
        int y = NextCenter.y + NextShape[i].y;

        if (
          x < 0 || x >= TETRIS_BOARD_WIDTH || y < 0
          || y >= TETRIS_BOARD_HEIGHT )
        {
            return true;
        }

        if ( t->Board[y][x] != 0 )
        {
            return true;
        }
    }

    return false;
}

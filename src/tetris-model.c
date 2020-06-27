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
    // ?…Œ?Š¸ë¦¬ìŠ¤ê°? ì§„í–‰?˜?Š” ë³´ë“œ
    tetris_cell_t Board[TETRIS_BOARD_HEIGHT][TETRIS_BOARD_WIDTH];

    // ?˜„?ž¬ ?–¨?–´ì§?ê³? ?žˆ?Š” ë¸”ë¡?˜ ? •ë³?
    // - ?œ„ì¹? ? •ë³?
    tetris_point_t CurBlockCenter;
    // - ëª¨ì–‘?— ????•œ ? •ë³?
    tetris_point_t CurBlockShape[4];
    // - ?˜„?ž¬ ë¸”ë¡
    int CurrentBlock;

    // ?˜„?ž¬ ë¸”ë¡?´ ?–¨?–´ì§?ê¸°ê¹Œì§??˜ ?‹œê°?
    float TimeLeftToAutoDrop;

    // ë¸”ë¡?´ ?–¨?–´ì§??Š” ê¸°ì?? ?‹œê°?
    float AutoDropTime;
    
    int score;
} tetris_t;

/////////////////////////////////////////////////////////////////////////////
// ?‚´ë¶? ?•¨?ˆ˜ ?„ ?–¸

// ë¸”ë¡?„ ?Š¤?°?•¨
static void SpawnNewBlock( tetris_t* t );

// ?žœ?¤?œ¼ë¡? ë¸”ë¡?„ ?ƒ?„±?•¨
static void GenerateRandomBlock( tetris_point_t arr[] );

// ì¶©ëŒ ê²??‚¬
// ë§Œì•½, ê²??‚¬?•˜? ¤?Š” ?œ„ì¹˜ì— ì¶©ëŒ?´ ë°œìƒ?•˜?Š” ê²½ìš° true ë°˜í™˜
static bool CheckCollision(
  tetris_t*      t,
  tetris_point_t NextCenter,
  tetris_point_t NextShape[] );

// ? ?ˆ˜ ?ž‘?—…
// ë³´ë“œ ? „ì²´ë?? ?Š¤ìº”í•´?„œ ? ?ˆ˜ê°? ?‚œ ë¸”ë¡?„ ë¹„ìš°ê³? ? „ì²? ë³´ë“œë¥? shift
static void ScoreProcess( tetris_t* t );

// êµ³ížˆ?Š” ?ž‘?—…
// ?˜„?ž¬ ë¸”ë¡?„ ë³´ë“œ?— ê³ ì •?‹œ?‚¤ê³? ?ƒˆë¡œìš´ ë¸”ë¡ ?• ?‹¹ë°›ìŒ
static void HoldBlock( tetris_t* t );

/////////////////////////////////////////////////////////////////////////////
// ?•¨?ˆ˜ ? •?˜

// ?…Œ?Š¸ë¦¬ìŠ¤ ?¸?Š¤?„´?Š¤ë¥? ë§Œë“ ?‹¤
tetris_hnd_t Tetris_Create()
{
    tetris_t* inst = calloc( 1, sizeof( tetris_t ) );
    return inst;
}

// ?…Œ?Š¸ë¦¬ìŠ¤ ê²Œìž„?„ ë¦¬ì…‹?•œ?‹¤
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

// ?˜„?ž¬ ë¸”ë¡?„ ?šŒ? „?‹œ?‚¨?‹¤ ( ë°©í–¥?„ ?¸?žë¡? ë°›ìŒ )
// ?šŒ? „ ?„±ê³µì‹œ true, ?‹¤?Œ¨?•˜ë©? false ë°˜í™˜
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

// ?˜„?ž¬ ë¸”ë¡?„ ?´?™?‹œ?‚¨?‹¤ ( ?•„?ž˜ ì¢? ?š° )
// Direction?´ 0?´ë©? ?•„?ž˜, -1?´ë©? ì¢?, 1?´ë©? ?š°
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

// ?˜„?ž¬ ë¸”ë¡?„ ?“œ?ž?•œ?‹¤
void Tetris_Drop( tetris_hnd_t h )
{
    while ( Tetris_Shift( h, 0 ) == true )
    {
    }
}

// ?—…?°?´?Š¸ ?•´ì¤??‹¤ ( delta ?‹œê°„ì„ ? „?‹¬ )
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
        // ÇÑ ÁÙ ¾¿ ±¸ÇØº½
        for (size_t k = 0; k < TETRIS_BOARD_WIDTH; k++)
        {
            mul = mul * t->Board[i][k];
        }
        // ¸¸¾à ±× ÁÙÀÇ ÀüÃ¼ °öÀÌ 0ÀÌ¸é ºóÄ­ÀÌ ÀÖ´Â °Í
        // ±×·¸Áö ¾ÊÀ¸¸é ºù°í!
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

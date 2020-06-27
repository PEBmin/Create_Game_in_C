#include <conio.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <windows.h>

#include "canvas.h"
#include "tetris-model.h"
#include "thread.h"

static char inp_buf_queue[128];
static int  inp_head;
static int  inp_tail;
static int  cursor_x, cursor_y;

void input_thread( void* nouse_ )
{
    for ( ;; )
    {
        char ch                 = getch();
        inp_buf_queue[inp_head] = ch;
        inp_head = inp_head + 1 == sizeof( inp_buf_queue ) ? 0 : inp_head + 1;
    }
}

static char input_next()
{
    if ( inp_tail == inp_head )
    {
        return 0;
    }

    char ret = inp_buf_queue[inp_tail];
    inp_tail = inp_tail + 1 == sizeof( inp_buf_queue ) ? 0 : inp_tail + 1;
    return ret;
}

static void RenderTetris( canvas_handle_t c, tetris_hnd_t h );

int main( void )
{
    tetris_hnd_t    tetris   = Tetris_Create();
    canvas_handle_t canv     = canvas_create( 118, 27 );
    thread_t        inp_thrd = thread_create( input_thread, NULL );
    clock_t         clk_beg, clk_end;
    float           interval = 0.066f; // 50ms
    float           elapsed  = interval;
    Tetris_Reset( tetris );

    // Temporarily implement view
    for ( ;; )
    {
        clk_beg = clock();

        // Simple input procedure
        for ( char c; c = input_next(), c != 0; )
        {
            switch ( c )
            {
            case 'a':
                Tetris_Shift( tetris, -1 );
                break;

            case 'd':
                Tetris_Shift( tetris, 1 );
                break;

            case 's':
                Tetris_Shift( tetris, 0 );
                break;

            case ' ':
                Tetris_Drop( tetris );
                break;
                
                case 'q':
                Tetris_Turn( tetris, 0 );
                break;
                
                case 'e':
                Tetris_Turn( tetris, 1 );
                break;
            }
        }

        // Update omok logic
        Tetris_Update( tetris, elapsed );

        // Render omok board
        RenderTetris( canv, tetris );

        // Wait for 50ms - elapsed time ...
        while ( true )
        {
            clk_end = clock();
            elapsed = ( clk_end - clk_beg ) / (double)( CLOCKS_PER_SEC );
            if ( elapsed >= interval )
                break;
            Sleep( 2 );
        }
    }
    return 0;
}

static void RenderTetris( canvas_handle_t c, tetris_hnd_t h )
{
    tetris_board_info_t t = Tetris_GetBoardInfo( h );
    canvas_clear( c );
    int ofstx = 6, ofsty = 5;

    for ( size_t y = 0; y < t.Height; y++ )
    {
        for ( size_t x = 0; x < t.Width; x++ )
        {
            // void canvas_draw_on(
            //   canvas_handle_t h, int x, int y, char const* ch );
            size_t      idx  = y * t.Width + x;
            char const* draw = t.Data[idx] == 0 ? "бр" : "бс";
            canvas_draw_on( c, ofstx + x * 2, ofsty + y, draw );
        }
    }
    
    printf(" current score: %d", t.score);
    
    for ( size_t i = 0; i < 4; i++ )
    {
        int x = t.CurBlockCenter.x + t.CurBlockShape[i].x;
        int y = t.CurBlockCenter.y + t.CurBlockShape[i].y;

        canvas_draw_on( c, ofstx + x * 2, ofsty + y, "в├" );
    }

    canvas_flush( c );
}
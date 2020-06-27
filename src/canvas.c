#include "canvas.h"
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>

// 픽셀 타입 정의
typedef struct
{
    char bin_[4]
} pixel_t;

struct canvas_struct
{
    int      width;
    int      height;
    pixel_t* buf;
};

typedef struct canvas_struct canvas_t;

// 새 캔버스를 생성한다 (초기화한다)
canvas_handle_t canvas_create( int width, int height )
{
    canvas_t* ret;
    ret         = malloc( sizeof( canvas_t ) );
    ret->width  = width + 1;
    ret->height = height;
    ret->buf    = malloc( sizeof( pixel_t ) * ret->width * height );
    memset( ret->buf, 0, sizeof( pixel_t ) * ret->width * height );

    HANDLE              consoleHandle = GetStdHandle( STD_OUTPUT_HANDLE );
    CONSOLE_CURSOR_INFO info;
    info.dwSize   = 100;
    info.bVisible = FALSE;
    SetConsoleCursorInfo( consoleHandle, &info );

    return (canvas_handle_t)ret;
}

// 캔버스를 할당 해제한다
void canvas_destroy( canvas_handle_t h )
{
    canvas_t* c = (canvas_t*)h;
    free( c->buf );
    free( c );
}

// 정해진 위치에 문자열 그리기
void canvas_draw_on( canvas_handle_t h, int x, int y, char const* ch )
{
    canvas_t* c = (canvas_t*)h;
    if ( x < 0 || y < 0 )
        return;
    int      ofst = y * c->width + x;
    pixel_t* p    = c->buf + ofst;

    // Overflow
    if ( ofst >= c->width * c->height )
        return;

    memset( p->bin_, 0, sizeof( p->bin_ ) );
    strncpy( p->bin_, ch, sizeof( p->bin_ ) - 1 );
}

void canvas_draw_string( canvas_handle_t h, int x, int y, char const* str )
{
    char bf[2];
    bf[1] = 0;

    while ( *str ) {
        bf[0] = *str++;
        canvas_draw_on( h, x++, y, bf );
    }
}

// 화면 지우기
void canvas_clear( canvas_handle_t h )
{
    canvas_t* c = (canvas_t*)h;
    memset( c->buf, 0, sizeof( *c->buf ) * c->width * c->height );
}

// 화면 내보내기
void canvas_flush( canvas_handle_t h )
{
    canvas_t* c = (canvas_t*)h;
    // 커서를 콘솔의 시작지점으로 옮기고
    HANDLE output = GetStdHandle( STD_OUTPUT_HANDLE );
    COORD  co;
    co.X = 0;
    co.Y = 0;
    SetConsoleCursorPosition( output, co );
    bool  bPrevIsFullWidth = false;
    char  buf_line[sizeof( *c->buf ) * c->width * c->height];
    char* buf_head = buf_line;
    *buf_head      = 0;

    // 글자를 하나씩 그리되
    // NULL문자는 공백 치환
    // 너비에 도달할 떄마다 개행
    for ( int y = 0; y < c->height; y++ ) {
        buf_head += strlen( buf_head );
        // Build line buffer before flush
        for ( int x = 0; x < c->width; x++ ) {
            // If previous character was full-width character, skip current
            if ( bPrevIsFullWidth ) {
                bPrevIsFullWidth = false;
                continue;
            }
            pixel_t* v = &c->buf[x + y * c->width];
            if ( v->bin_[0] == 0 ) {
                v->bin_[0] = ' ';
                v->bin_[1] = 0;
            }

            strcat( buf_head, v->bin_ );

            if ( *(uint32_t*)v > 0xff ) {
                bPrevIsFullWidth = true;
            }
        }
        strcat( buf_head, "\n" );
    }
    printf( buf_line );
}

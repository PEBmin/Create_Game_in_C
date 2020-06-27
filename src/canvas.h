#pragma once

// 사용자는 이 포인터(핸들)를 통해서만
// Canvas에 접근할 수 있음
typedef void* canvas_handle_t;

// 새 캔버스를 생성한다 (초기화한다)
canvas_handle_t canvas_create( int width, int height );

// 캔버스를 할당 해제한다
void canvas_destroy( canvas_handle_t h );

// 정해진 위치에 문자 그리기. 유니코드 문자 대응 위해 문자열 형태
void canvas_draw_on( canvas_handle_t h, int x, int y, char const* ch );

// 정해진 위치에 ASCII 문자열 그리기. 유니코드는 사용 못함!
void canvas_draw_string( canvas_handle_t h, int x, int y, char const* str );

// 화면 지우기
void canvas_clear( canvas_handle_t h );

// 화면 내보내기
void canvas_flush( canvas_handle_t h );
#pragma once
#include <stdbool.h>

typedef void* thread_t;
typedef void ( *thread_fnc_t )( void* );
typedef void* mutex_t;

thread_t thread_create( thread_fnc_t callback, void* argument );
bool     thread_join( thread_t thr );

mutex_t mutex_create();
void    mutex_lock( mutex_t m );
void    mutex_unlock( mutex_t m );
bool    mutex_try_lock( mutex_t m );
void    mutex_destroy( mutex_t m );
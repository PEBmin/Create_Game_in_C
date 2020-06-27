#include "thread.h"
#include <windows.h>

typedef struct th_adapter {
    thread_fnc_t exec;
    void*        obj;
} th_adapter_t;

static DWORD WINAPI THREAD_ADAPTER_FUNCTION( LPVOID pvarg )
{
    th_adapter_t volatile* arg = (th_adapter_t volatile*)pvarg;
    thread_fnc_t           fnc = arg->exec;
    void*                  obj = arg->obj;

    // Release data
    free( obj );

    // Execute task
    fnc( obj );

    return 0;
}

thread_t thread_create( thread_fnc_t callback, void* argument )
{
    th_adapter_t* pdata = (th_adapter_t*)malloc( sizeof( th_adapter_t ) );
    *pdata              = ( th_adapter_t ) {
        .exec = callback,
        .obj  = argument,
    };
    HANDLE ret = CreateThread( NULL, 0, THREAD_ADAPTER_FUNCTION, pdata, 0, NULL );
    return (thread_t)ret;
}

bool thread_join( thread_t thr )
{
    return WaitForSingleObject( thr, INFINITE );
}

mutex_t mutex_create()
{
    return CreateMutex( NULL, false, NULL );
}

void mutex_lock( mutex_t m )
{
    WaitForSingleObject( m, INFINITE );
}

void mutex_unlock( mutex_t m )
{
    ReleaseMutex( m );
}

bool mutex_try_lock( mutex_t m )
{
    return WaitForSingleObject( m, 0 );
}

void mutex_destroy( mutex_t m )
{
    CloseHandle( m );
}

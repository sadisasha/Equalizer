
/* Copyright (c) 2005-2006, Stefan Eilemann <eile@equalizergraphics.com> 
   All rights reserved. */

#include "timedLock.h"

#include <eq/base/debug.h>

#include <pthread.h>
#include <errno.h>
#include <sys/timeb.h>

#ifdef WIN32_VC
#  define timeb _timeb
#  define ftime _ftime
#endif

using namespace std;

namespace eqBase
{
class TimedLockPrivate
{
public:
    pthread_mutex_t mutex;
    pthread_cond_t  cond;
};

TimedLock::TimedLock()
        : _data( new TimedLockPrivate )
        , _locked( false )
{
    int error = pthread_mutex_init( &_data->mutex, NULL );
    if( error )
    {
        EQERROR << "Error creating pthread mutex: " << strerror(error) << endl;
        return;
    }
    error = pthread_cond_init( &_data->cond, NULL );
    if( error )
    {
        EQERROR << "Error creating pthread condition: " << strerror( error )
                << endl;
        pthread_mutex_destroy( &_data->mutex );
        return;
    }
}

TimedLock::~TimedLock()
{
    pthread_mutex_destroy( &_data->mutex );
    pthread_cond_destroy( &_data->cond );
    delete _data;
    _data = 0;
}

bool TimedLock::set( const uint32_t timeout )
{
    pthread_mutex_lock( &_data->mutex );

    bool acquired = true;
    while( _locked )
    {
        if( timeout )
        {
            timespec ts = {0};
            if( timeout > 0 )
            {
                ts.tv_sec  = static_cast<int>( timeout / 1000 );
                ts.tv_nsec = (timeout - ts.tv_sec*1000) * 1000000;
            }

            timeb tb;
            ftime( &tb );
            ts.tv_sec  += tb.time;
            ts.tv_nsec += tb.millitm * 1000000;
            
            int error = pthread_cond_timedwait( &_data->cond, &_data->mutex, &ts );
            if( error == ETIMEDOUT )
            {
                acquired = false;
                break;
            }
        }
        else
            pthread_cond_wait( &_data->cond, &_data->mutex );
    }

    if( acquired )
    {
        EQASSERT( !_locked );
        _locked = true;
    }

    pthread_mutex_unlock( &_data->mutex );
    return acquired;
}

void TimedLock::unset()
{
    pthread_mutex_lock( &_data->mutex );
    _locked = false;
    pthread_cond_signal( &_data->cond );
    pthread_mutex_unlock( &_data->mutex );
}


bool TimedLock::trySet()
{
    pthread_mutex_lock( &_data->mutex );
    
    bool acquired = false;
    if( _locked )
    {
        _locked  = true;
        acquired = true;
    }

    pthread_mutex_unlock( &_data->mutex );
    return acquired;
}


bool TimedLock::test()
{
    return _locked;
}
}

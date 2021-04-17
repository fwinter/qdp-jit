/*! @file
 * @brief Timer support
 *
 * A stopwatch like timer.
 */


#include "qdp.h"
#include<sys/time.h>

namespace QDP {

StopWatch::StopWatch( bool _device_sync )
{
  stoppedP=false;
  startedP=false;
  device_sync = _device_sync;
}

StopWatch::~StopWatch() {}

void StopWatch::reset() 
{
  startedP = false;
  stoppedP = false;
}

void StopWatch::start() 
{
  if (device_sync)
    jit_util_sync_copy();

  int ret_val;
  ret_val = gettimeofday(&t_start, NULL);
  if( ret_val != 0 ) 
  {
    QDPIO::cerr << __func__ << ": gettimeofday failed in StopWatch::start()" << endl;
    QDP_abort(1);
  }
  startedP = true;
  stoppedP = false;
}

void StopWatch::stop() 
{
  if (device_sync)
    jit_util_sync_copy();

  if( !startedP ) 
  { 
    QDPIO::cerr << __func__ << ": attempting to stop a non running stopwatch in StopWatch::stop()" << endl;
    QDP_abort(1);
  }

  int ret_val;
  ret_val = gettimeofday(&t_end, NULL);
  if( ret_val != 0 ) 
  {
    QDPIO::cerr << __func__ << ": gettimeofday failed in StopWatch::end()" << endl;
    QDP_abort(1);
  }
  stoppedP = true;
}

double StopWatch::getTimeInMicroseconds() 
{
  long usecs=0;
  if( startedP && stoppedP ) 
  { 
    if( t_end.tv_sec < t_start.tv_sec ) 
    { 
      QDPIO::cerr << __func__ << ": critical timer rollover" << endl;
//      QDP_abort(1);
      usecs = 0.0;
    }
    else 
    { 
      usecs = (t_end.tv_sec - t_start.tv_sec)*1000000;

      if( t_end.tv_usec < t_start.tv_usec ) 
      {
	usecs -= 1000000;
	usecs += 1000000+t_end.tv_usec - t_start.tv_usec;
      }
      else 
      {
	usecs += t_end.tv_usec - t_start.tv_usec;
      }
    }
  }
  else 
  {
    QDPIO::cerr << __func__ << ": either stopwatch not started, or not stopped" << endl;
    QDP_abort(1);
  }

  return (double)usecs;
}
    
double StopWatch::getTimeInSeconds()  
{
  long secs=0;
  long usecs=0;
  if( startedP && stoppedP ) 
  { 
    if( t_end.tv_sec < t_start.tv_sec ) 
    { 
      QDPIO::cerr << __func__ << ": critical timer rollover" << endl;
//      QDP_abort(1);
      usecs = 0.0;
    }
    else 
    { 
      secs = t_end.tv_sec - t_start.tv_sec;

      if( t_end.tv_usec < t_start.tv_usec ) 
      {
	secs -= 1;
	usecs = 1000000;
      }
      usecs += t_end.tv_usec - t_start.tv_usec;
    }
  }
  else 
  {
    QDPIO::cerr << __func__ << ": either stopwatch not started, or not stopped" << endl;
    QDP_abort(1);
  }

  return (double)secs + ((double)usecs / 1e6);
}


} // namespace QDP;

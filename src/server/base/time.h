#ifndef __TIME_H
#define __TIME_H

#include <sys/time.h>
#include <stdio.h>

#include <ostream>

#include "base/types.h"

namespace vodeox
{

class time
{
    //this has a number of miscroseconds since 1970
    uint64   m_time;

 public:

 time() :
    m_time(0)
    {
    }
   
 time(const time& t) :
    m_time(t.m_time)
    {

    }
          
 time(uint64 t) : 
   m_time(t)
    {
    }

   const time& operator=(const time& t)
   {
       m_time = t.usec();
       return *this;
   }
    
   static time now()
    {
        timeval t;
        if (0 != gettimeofday(&t, NULL))
            fprintf(stderr, "something bad happened, we couldn't get a timestamp %s\n", __FUNCTION__);

        return time(t.tv_sec * 1000000 + t.tv_usec);        
    }

    const time& operator+(const time& t)
    {
        m_time += t.usec();
        return *this;
    }

    const time& operator-(const time& t)
    {
        m_time -= t.usec();
        return *this;
    }

    bool operator>(const time& t)
    {
        return m_time > t.usec();
    }

    bool operator==(const time& t)
    {
        return m_time == t.usec();
    }

    uint64 usec() const
    {
        return m_time;
    }

    uint64 msec() const
    {
        return m_time / 1000;
    }

    uint64 sec() const
    {
        return m_time / 1000000;
    }

};

std::ostream &
    operator<<(std::ostream & ostr, time const & t);

}

#endif

#include <pthread.h>

namespace vodeox
{

class mutex
{
    pthread_mutex_t m_pth_mutex;
 public:
    mutex()
    {
        if (0 != pthread_mutex_init(&m_pth_mutex, NULL))
        {
            fprintf(stderr, "couldn't create pthread mutex, aborting...");
            abort();
        }
    }

    virtual ~mutex()
    {
        pthread_mutex_destroy(&m_pth_mutex);
    }

    void lock()
    {
        if (0 != pthread_mutex_lock(&m_pth_mutex))
            fprintf(stderr, "couldn't lock mutex");
    }
    void unlock()
    {
        if (0 != pthread_mutex_unlock(&m_pth_mutex))
            fprintf(stderr, "couldn't unlock mutex");
    }

    void trylock()
    {
        if (0 != pthread_mutex_trylock(&m_pth_mutex))
            fprintf(stderr, "couldn't lock mutex");
    }

};

class scoped_lock
{
    mutex&  m_mutex;
 public:

    scoped_lock(mutex& mutex) : m_mutex(mutex)  { m_mutex.lock();}
    virtual ~scoped_lock(){ m_mutex.unlock(); }
};

} //namespace vodeox

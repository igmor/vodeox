#include <pthread.h>
#include <errno.h>

namespace vodeox
{

class mutex
{
    friend class condition_variable;
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

class condition_variable
{
    pthread_cond_t m_cond;

 public:
    condition_variable() 
    {
        pthread_cond_init(&m_cond, NULL);
    }

    virtual ~condition_variable() 
    {
        if (0 != pthread_cond_destroy(&m_cond))
            fprintf(stderr, "error in %s\n", __FUNCTION__);
    } 

    void wait(vodeox::mutex& mutex)
    {
        if (0 != pthread_cond_wait(&m_cond, &mutex.m_pth_mutex))
            fprintf(stderr, "error in %s\n", __FUNCTION__);        
    }

    void timed_wait(vodeox::mutex& mutex, const struct timespec *abstime)
    {
        int ret = pthread_cond_timedwait(&m_cond, &mutex.m_pth_mutex, abstime);
        if (0 != ret && ETIMEDOUT != ret)
            fprintf(stderr, "error in %s\n", __FUNCTION__);                
    }

    void notify()
    {
        if (0 != pthread_cond_signal(&m_cond))
            fprintf(stderr, "error in %s\n", __FUNCTION__);                
    }

    void notify_all()
    {
        if (0 != pthread_cond_broadcast(&m_cond))
            fprintf(stderr, "error in %s\n", __FUNCTION__);
    }

};

class scoped_lock
{
    mutex&  m_mutex;
 public:

    scoped_lock(mutex& mutex) : m_mutex(mutex)  { m_mutex.lock();}
    virtual ~scoped_lock(){ m_mutex.unlock(); }

    //to be able to control the scope of the mutex locking
    void unlock() { m_mutex.unlock(); }
};

class thread
{
      public:
            thread() 
            { 
            }

            void start(void* arg) 
            { 
                if (0 != pthread_create(&m_thread, NULL, thread::thread_function, arg))
                    fprintf(stderr, "couldn't create logger thread, error=%d, %s", errno, __FUNCTION__);
            }
            
            void join() 
            { 
                pthread_join(m_thread, NULL); 
            }
             
            virtual void run() = 0;
 
      private:
            static void* thread_function(void* arg)
            {
                reinterpret_cast<thread*>(arg)->run();
                return NULL;
            }

            void execute() { run(); }
      private:
            pthread_t m_thread;
};

} //namespace vodeox

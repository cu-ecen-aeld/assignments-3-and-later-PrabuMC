 #define _GNU_SOURCE 
#include "threading.h"
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
 
#include <pthread.h>

// Optional: use these functions to add debug or error prints to your application
#define DEBUG_LOG(msg,...)
//#define DEBUG_LOG(msg,...) printf("threading: " msg "\n" , ##__VA_ARGS__)
#define ERROR_LOG(msg,...) printf("threading ERROR: " msg "\n" , ##__VA_ARGS__)
typedef struct thread_data Thread_data;
int thread_ret = 0;
void* threadfunc(void* thread_param)
{

    // TODO: wait, obtain mutex, wait, release mutex as described by thread_data structure
    // hint: use a cast like the one below to obtain thread arguments from your parameter
    //struct thread_data* thread_func_args = (struct thread_data *) thread_param;
    Thread_data *data = (Thread_data *) thread_param;
    usleep(data->wait_to_obtain_ms * 1000);
    pthread_mutex_lock(data->mtx);
    usleep(data->wait_to_release_ms * 1000);
    pthread_mutex_unlock(data->mtx);
    data->thread_complete_success = true;
    return thread_param;
}


bool start_thread_obtaining_mutex(pthread_t *thread, pthread_mutex_t *mutex,int wait_to_obtain_ms, int wait_to_release_ms)
{
    /**
     * TODO: allocate memory for thread_data, setup mutex and wait arguments, pass thread_data to created thread
     * using threadfunc() as entry point.
     *
     * return true if successful.
     *
     * See implementation details in threading.h file comment block
     */
    if (thread == NULL || mutex == NULL)
    	return false;
    //*mutex = PTHREAD_MUTEX_INITIALIZER;

    Thread_data *data = (Thread_data *)malloc(sizeof(Thread_data));
    if (data == NULL)
    	return false;
    data->mtx = mutex;
    data->wait_to_obtain_ms = wait_to_obtain_ms;
    data->wait_to_release_ms = wait_to_release_ms;
    int ret = pthread_create(thread, NULL, threadfunc, (void *)data);
    if (ret != 0) {
    	free(data);
    	return false;
    } else
	return true;
}


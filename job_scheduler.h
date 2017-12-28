#ifndef __JOB_SCHEDULER__
#define __JOB_SCHEDULER__

#include <pthread.h>
#include <stdlib.h>
#include "job.h"

#define JOBS_AMOUNT 30
#define THREADS_AMOUNT 10

typedef struct queue{
    int size;
    int amount_of_jobs;
    int position;
    job* my_jobs;
}queue;

typedef struct job_scheduler{
    pthread_t * threads;
    pthread_mutex_t mut_get_a_job;
    pthread_cond_t cond_get_a_job;

    pthread_mutex_t mut_finished_f;
    pthread_cond_t cond_finished_f;

    pthread_mutex_t mut_heap_update;
    pthread_cond_t cond_heap_update;

    int heap_busy;
    int finished_jobs;

    int exit_programm;
    queue* my_queue;
    int threads_amount;
}job_scheduler;

queue* create_queue();
job_scheduler* initialize_scheduler();
void submit_job(queue* my_queue,job* myjob);
void clean_job_table(queue* my_queue);
void destroy_job_scheduler(job_scheduler** myjob_scheduler);


#endif

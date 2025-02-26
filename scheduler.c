#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <pthread.h>
#include "math.h"
#include <sys/types.h>
#include <string.h>

// Student Name: Riya Rawat
// Student ID: 101193396

#define NUM_THREADS 4
#define MAX(x, y) (((x) > (y)) ? (x) : (y))
#define MIN(x, y) (((x) < (y)) ? (x) : (y))

void prio_less_than_100(int index, int thread_num);
void prio_less_than_130(int index, int thread_num);
void prio_less_than_140(int index, int thread_num);
void *thread_function(void *arg);
void *single_thread_function(void *arg);

struct buffer_t
{
    pid_t pid;
    int static_prio;
    int dynamic_prio;
    double remain_time;
    double time_slice;
    double accu_time_slice;
    int last_cpu;
    char sched_type[10];
};
struct buffer_t buffers[NUM_THREADS];
struct buffer_t RQ0[NUM_THREADS][10];
struct buffer_t RQ1[NUM_THREADS][10];
struct buffer_t RQ2[NUM_THREADS][10];
int RQ_0[3];
int RQ_1[3];
int RQ_2[3];

int thread_finished = 0;

int main()
{
    int res;
    pthread_t a_thread[NUM_THREADS];
    pthread_t single_thread;
    void *thread_result;
    int lots_of_threads;
    pthread_attr_t thread_attr;
    struct sched_param scheduling_value;

    // reading from the file
    FILE *file;
    char buf[1024];

    // open thie file
    file = fopen("processes.txt", "r");
    if (NULL == file)
    {
        printf("file can't be opened \n");
    }

    printf("content of this file are \n");

    int priority, exec_time;
    int i = 0;
    int q = 0;

    //looping through the 20 processes
    while (i < 20)
    {   q = 1 % 4;
        fscanf(file, "%[^,],%d,%d\n", buf, &priority, &exec_time);
        
        if (priority <= 100)
        {
            RQ0[q][RQ_0[q]].static_prio = priority;
            RQ0[q][RQ_0[q]].remain_time = exec_time;
            strcpy(RQ0[q][RQ_0[q]].sched_type, buf);
            RQ_0[q]++;
        }
        else if (priority <= 130)
        {
            RQ1[q][RQ_1[q]].static_prio = priority;
            RQ1[q][RQ_1[q]].remain_time = exec_time;
            strcpy(RQ1[q][RQ_1[q]].sched_type, buf);
            RQ_1[q]++;
        }
        else if (priority <= 140)
        {
            RQ2[q][RQ_2[q]].static_prio = priority;
            RQ2[q][RQ_2[q]].remain_time = exec_time;
            strcpy(RQ2[q][RQ_2[q]].sched_type, buf);
            RQ_2[q]++;
        }

        i++;
    }
    fclose(file);

    // create consumer threads
    for (lots_of_threads = 0; lots_of_threads < NUM_THREADS; lots_of_threads++)
    {

        res = pthread_create(&(a_thread[lots_of_threads]), NULL, thread_function, (void *)&lots_of_threads);
        if (res != 0)
        {
            perror("Thread creation failed");
            exit(EXIT_FAILURE);
        }
        usleep(500);
    }

    sleep(1);

    printf("Waiting for consumer threads to finish...\n");
    for (lots_of_threads = NUM_THREADS - 1; lots_of_threads >= 0; lots_of_threads--)
    {
        res = pthread_join(a_thread[lots_of_threads], &thread_result);
        if (res == 0)
        {
            printf("Picked up a thread\n");
        }
        else
        {
            perror("pthread_join failed");
        }
    }

    exit(EXIT_SUCCESS);
}

void *thread_function(void *arg)
{
    int thread_num = *(int *)arg;
    // looping through the queues if they are not empty
    while (RQ_0[thread_num] != 0 || RQ_1[thread_num] != 0 || RQ_2[thread_num] != 0)
    {
        //highest priority for RQ0
        if (RQ_0[thread_num] > 0)
        {
            int max_prio = 100;
            int index = 0;
            for (int i = 0; i < RQ_0[thread_num]; i++)
            {
                if (RQ0[thread_num][i].static_prio < max_prio)
                {
                    max_prio = RQ0[thread_num][i].static_prio;
                    index = i;
                }
            }
            if (max_prio < 100)
            {
                prio_less_than_100(index, thread_num);
            }
            else
            {
                RQ_0[thread_num] = 0;
            }
        }
        //highest priority for RQ1
        else if (RQ_1[thread_num] > 0)
        {
            int max_prio = 130;
            int index = 0;
            for (int i = 0; i < RQ_1[thread_num]; i++)
            {
                if (RQ1[thread_num][i].static_prio < max_prio)
                {
                    max_prio = RQ1[thread_num][i].static_prio;
                    index = i;
                }
            }
            if (max_prio < 130)
            {
                prio_less_than_130(index, thread_num);
            }
            else
            {
                RQ_0[thread_num] = 0;
            }
        }
        //highest priority for RQ2
        else
        {
            int max_prio = 140;
            int index = 0;
            for (int i = 0; i < RQ_2[thread_num]; i++)
            {
                if (RQ2[thread_num][i].static_prio < max_prio)
                {
                    max_prio = RQ2[thread_num][i].static_prio;
                    index = i;
                }
            }
            if (max_prio < 140)
            {
                prio_less_than_140(index, thread_num);
            }
            else
            {
                RQ_2[thread_num] = 0;
            }
        }
    }

    pthread_exit(NULL);
}

void prio_less_than_100(int x, int thread_num)
{
    RQ0[thread_num][x].static_prio = 100;
    // NORMAL
    if (strcmp(RQ0[thread_num][x].sched_type, "NORMAL") == 0)
    {
        int rand_time;
        //generating a number i the range from 1 to 10
        rand_time = (rand() % 10) * 5;
        RQ2[thread_num][x].remain_time -= rand_time;
        // starting execution time
        usleep(rand_time); 
        int bonus = 10;
        //calculations for dynamic priority
        RQ0[thread_num][x].dynamic_prio = MAX(100, MIN(RQ0[thread_num][x].static_prio - bonus + 5, 139));
        //printing the thread 
        printf("Execution of Thread%d:  NORMAL: %dms, RQ0 Priority: %d, Dynamic Priority: %d\n",
               thread_num, rand_time, RQ0[thread_num][x].static_prio, RQ0[thread_num][x].dynamic_prio);


        // when there is certain remain time after execution of thread 
        if (RQ0[thread_num][x].remain_time > 0)
        {
            exit(EXIT_SUCCESS);
        }
        else
        {
            exit(EXIT_SUCCESS);
        }
    }
    // RR
    else if (strcmp(RQ0[thread_num][x].sched_type, "RR") == 0)
    {   //calculations for quantum or time slice
        RQ0[thread_num][x].time_slice = (140 - RQ0[thread_num][x].static_prio) * 20;
        usleep(RQ0[thread_num][x].time_slice);
        RQ0[thread_num][x].remain_time -= RQ0[thread_num][x].time_slice;
        //printing the thread
        printf("Execution of Thread%d: Execution time for RR: %fms, RQ0 (Static) Priority: %d\n", thread_num,
               RQ0[thread_num][x].time_slice, RQ0[thread_num][x].static_prio);
        if (RQ0[thread_num][x].remain_time <= 0)
        {
            RQ_0[thread_num]--;
        }
    }
    // FIFO
    else
    {
        usleep(RQ0[thread_num][x].remain_time);
        //printing the thread
        printf("Execution of Thread%d: FIFO: RQ0 Priority: %d, Execution Time: %fms\n", thread_num,
               RQ0[thread_num][x].static_prio, RQ0[thread_num][x].remain_time);
        RQ_0[thread_num]--;
    }
}

void prio_less_than_130(int y, int thread_num)
{
    RQ1[thread_num][y].static_prio = 140;
    // NORMAL
    if (strcmp(RQ1[thread_num][y].sched_type, "NORMAL") == 0)
    {   //generating a number i the range from 1 to 10
        int rand_time;
        rand_time = (rand() % 10) * 6;
        RQ2[thread_num][y].remain_time -= rand_time;
         // starting execution time
        usleep(rand_time); 
        int bonus = 10;
        //calcualtions for dynamic priority
        RQ1[thread_num][y].dynamic_prio = MAX(100, MIN(RQ1[thread_num][y].static_prio - bonus + 5, 139));
        //printing the thread
        printf("Execution of Thread%d: Execution time for NORMAL: %dms RQ1 Priority: %d, Dynamic Priority: %d\n",
               thread_num, rand_time, RQ1[thread_num][y].static_prio, RQ1[thread_num][y].dynamic_prio);

        // when there is certain remain time after execution of thread 
        if (RQ1[thread_num][y].remain_time > 0)
        { 
            exit(EXIT_SUCCESS);
        }
        else
        {
           
            exit(EXIT_SUCCESS);
        }
    }
    // RR
    else if (strcmp(RQ1[thread_num][y].sched_type, "RR") == 0)
    {   //calculations for quantum or time slice
        if (RQ1[thread_num][y].static_prio < 120)
        {
            RQ1[thread_num][y].time_slice = (140 - RQ0[thread_num][y].static_prio) * 20;
        }
        else
        {
            RQ1[thread_num][y].time_slice = (140 - RQ1[thread_num][y].static_prio) * 5;
        }
        usleep(RQ1[thread_num][y].time_slice);
        RQ1[thread_num][y].remain_time -= RQ2[thread_num][y].time_slice;
        //printing the thread
        printf("Execution of Thread%d: Execution time for RR: %0.1f ,RQ1 Priority: %d\n", thread_num,
               RQ1[thread_num][y].time_slice, RQ1[thread_num][y].static_prio);
    }
    // FIFO
    else
    {   //printing the thread
        printf("Execution of Thread%d: FIFO: RQ1 Priority: %d, Execution Time: %fms\n", thread_num,
               RQ1[thread_num][y].static_prio, RQ1[thread_num][y].remain_time);
        usleep(RQ1[thread_num][y].remain_time);
    }
    RQ_1[thread_num]--;
}

void prio_less_than_140(int z, int thread_num)
{
    RQ2[thread_num][z].static_prio = 145;
    // NORMAL
    if (strcmp(RQ2[thread_num][z].sched_type, "NORMAL") == 0)
    {
        int rand_time;
        rand_time = (rand() % 10) * 6;
        RQ2[thread_num][z].remain_time -= rand_time;
        //starting execution time
        usleep(rand_time); 
        int bonus = 10;
        //calculations for dynamic priority
        RQ2[thread_num][z].dynamic_prio = MAX(100, MIN(RQ2[thread_num][z].static_prio - bonus + 5, 139));
        //printing the thread
        printf("Execution of Thread%d: Execution time for NORMAL: %dms RQ2 Priority: %d, Dynamic Priority: %d\n",
               thread_num, rand_time, RQ2[thread_num][z].static_prio, RQ2[thread_num][z].dynamic_prio);

        if (RQ2[thread_num][z].remain_time > 0)
        { 
            exit(EXIT_SUCCESS);
        }
        else
        {

            exit(EXIT_SUCCESS);
        }
    }
    // FIFO OR RR
    else
    {   //printing the thread
        printf("Execution of Thread%d: FIFO: RQ2 Priority: %d, Execution Time: %fms\n", thread_num,
               RQ2[thread_num][z].static_prio, RQ2[thread_num][z].remain_time);
        usleep(1000);
    }
    RQ_2[thread_num]--;
}
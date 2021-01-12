#define _CRT_SECURE_NO_WARNINGS
#define DEBUG 0
#define SUCCESS 0
#include <pthread.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <time.h>

const char *inf = { "input.txt" };
const char *ouf = { "output.txt" };
const char *time_ouf = { "time.txt" };

typedef struct th_descr {
    int id;
    int task_amount;
    int task_arr[100];
    int result;
} thread_description;

pthread_t *threads;
pthread_mutex_t mutex;
thread_description *descr_arr;
int thrds_num, descr_num, num;

int error(const char *err);
int get_args();
void structures_init();
void structures_deinit();
void task_divide();
void thread_creation();
void *thread_entry(void *param);
int separations_find(int n, int k);
void get_time();

int main(void) {
    int i;

    if (get_args())
        return error("fopen failed.");
    
    structures_init();            

    i = 0;
    while (i < thrds_num){
        descr_arr[i].id = i;
        descr_arr[i++].task_amount = 0;
    }

    task_divide();
    thread_creation();
    get_time();

    structures_deinit();
    return 0;
}

int error(const char *err) {
    printf("ERROR: %s", err);
    return -1;
}

int get_args(void) {
    FILE *f;
    char line[20];

    f = fopen(inf, "rb");
    if (f == NULL)
        return 1;

    fscanf(f, "%s", line);
    thrds_num = atoi(line);
    fscanf(f, "%s", line);
    num = atoi(line);

    if (DEBUG)
        printf("Input: thrds_num == %d, num == %d\n", thrds_num, num);

    fclose(f);
    return 0;
}

void structures_init() {
    descr_arr = (thread_description*)malloc(sizeof(thread_description) * thrds_num);
    pthread_mutex_init(&mutex, 0);
}

void structures_deinit() { 
    free(descr_arr);
    free(threads);
    pthread_mutex_destroy(&mutex);
}

void print_divide_debug_info() {
    int i, j;

    for (i = 0; i < thrds_num && descr_arr[i].task_amount != 0; i++) {
        printf("\nthread #%d job: ", i);
        for (j = 0; j < descr_arr[i].task_amount; j++) {
            printf(" %d", descr_arr[i].task_arr[j]);
        }
        printf("\n");
    }

    if (thrds_num <= num) printf("\nAll %d threads got job!\n", thrds_num);
    else printf("\nOnly %d threads got job!\n", num);
}

void task_divide(void) {
    int border_l, border_u, thread_ind, index;
    bool dir;

    if (DEBUG)
        printf("\nNow in -> task_divide\n");

    dir = border_l = 1;
    border_u = num;
    thread_ind = index = 0;

    for (int cntr = num; cntr;) {
        switch (dir)
        {
        case 0:
            descr_arr[thread_ind].task_amount++;
            descr_arr[thread_ind].task_arr[index] = border_l++;
            thread_ind++;
            break;
        case 1:
            descr_arr[thread_ind].task_amount++;
            descr_arr[thread_ind].task_arr[index] = border_u--;
            thread_ind++;
            break;
        }

        cntr--;

        if (thrds_num == thread_ind) {
            thread_ind = 0;
            index++;
            dir = !dir;
        }
    }

    if (DEBUG)
        print_divide_debug_info();

    if (thrds_num <= num)
        descr_num = thrds_num;
    else
        descr_num = num;
}

void thread_creation(void) {
    int ind, status;

    threads = (pthread_t*)malloc(sizeof(pthread_t) * descr_num);

    for (ind = 0; ind < descr_num; ind++) {
        status = pthread_create(&threads[ind], 0, thread_entry, &descr_arr[ind]);
        if (status != SUCCESS)
            exit(printf("\nthread_creation ERROR!\n"));
    }
}

int separations_find(int n, int k) {
    if ((0 < k) && (k <= n))
        return(separations_find(n, k - 1) + separations_find(n - k, k));
    else if (k > n)
        return(separations_find(n, n));
    else if ((n == 0) && (k == 0))
        return 1;
    else if ((n != 0) && (k == 0))
        return 0;
    else
        exit(printf("\nERROR: separations_find\n"));
}

void *thread_entry(void *param) {
    thread_description tmp_job;
    int res, i, n, k;

    tmp_job = *(thread_description*)param;
    res = i = 0;

    if (DEBUG)
        printf("Thread #%d created. Got mutex.\n", tmp_job.id);

    pthread_mutex_lock(&mutex);

    for (; i < tmp_job.task_amount; i++) {
        n = num - tmp_job.task_arr[i];
        k = tmp_job.task_arr[i];
        if (DEBUG)
            printf("Thread #%d with n: %d, k: %d\n", tmp_job.id, n, k);
        res += separations_find(n, k);
    }
    (*(thread_description*)param).result = res;

    if (DEBUG)
        printf("Thread #%d finished. Free mutex.\n", tmp_job.id);

    pthread_mutex_unlock(&mutex);
}

void join_all_threads(void){
    int i = 0;
    while (i < descr_num) {
        pthread_join(threads[i], 0);
        i++;
    }
}

void get_time(void) {
    FILE* f;
    double time1, time2;
    double res_time;
    int i = 0, res = 0;

    time1 = (double)clock();
    join_all_threads();
    time2 = (double)clock();

    res_time = (time2 - time1) * 1000;
    res_time /= (double)CLOCKS_PER_SEC;

    f = fopen(ouf, "wb");
    if (f == NULL) {
        exit(printf("output.txt is not found."));
    }
    for (i = 0; i < descr_num; i++)
        res += descr_arr[i].result;
    if (DEBUG)
        printf("\n\n%d threads P(%d) = %d, time consumed : %lf\n", descr_num, num, res, res_time);
    res--;
    fprintf(f, "%d\n%d\n%d", thrds_num, num, res);
    fclose(f);

    f = fopen(time_ouf, "wb");
    fprintf(f, "%lf", res_time);
    fclose(f);
}

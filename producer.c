#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <errno.h>

#define SET_SEMOP(num, s_num, op, flag)     \
    semops[num].sem_num = s_num;            \
    semops[num].sem_op  = op;               \
    semops[num].sem_flg = flag;

#define INIT        semops + 0
#define WRITE       semops + 6
#define P_SEM2      semops + 9
#define P_MUTE      semops + 10
#define V_MUTE      semops + 11
#define V_SEM1      semops + 12
#define V_TRASH     semops + 13
#define V_PROD      semops + 14
#define CHECK_C     semops + 15
#define ENTRY       semops + 7

#define D if (0)

void clear_rest (int read_s, char *mem_p);

size_t buff_size = 1024;
int sem_key = 42;
int shm_key = 3802;

int main(int argc, char ** argv) {
    if (argv == NULL || argc != 2) {
        printf("Ayayay!\n");
        return 0;
    }

    int input_fd = open(argv[1], O_RDONLY);     //open file to read and prepare for copy
    if (input_fd == -1) { 
        printf("Cant open file\n");
        return 0;
    }

    int read_s = 1;
    char *buff = calloc(buff_size, sizeof(char));
    int sems = semget(sem_key, 9, IPC_CREAT | 0644);
    int shared_mem = shmget(shm_key, buff_size, IPC_CREAT | 0644);
    char* mem_p = shmat(shared_mem, NULL, 0);

    struct sembuf semops[17];
    SET_SEMOP(0, 0, 0,   IPC_NOWAIT);//init                        
    SET_SEMOP(1, 0, 1,   IPC_NOWAIT);                             
    SET_SEMOP(2, 1, 1,   IPC_NOWAIT);                          
    SET_SEMOP(3, 2, 1,   IPC_NOWAIT);                             
    SET_SEMOP(4, 3, 1,   IPC_NOWAIT);                               
    SET_SEMOP(5, 5, 1,   IPC_NOWAIT);
    SET_SEMOP(6, 2, -1,  SEM_UNDO);  //write
    SET_SEMOP(7, 8, -1, SEM_UNDO | IPC_NOWAIT);  //check_consumer
    SET_SEMOP(8, 8, 1,  SEM_UNDO | IPC_NOWAIT);                   
    SET_SEMOP(9, 5, -1,  0);         //p_sem2
    SET_SEMOP(10, 3, -1,  SEM_UNDO);  //p_mute
    SET_SEMOP(11, 3, 1,   SEM_UNDO);  //v_mute
    SET_SEMOP(12, 4, 1,  0);         //v_sem1
    SET_SEMOP(13, 6, 1,  IPC_NOWAIT);//v_trash
    SET_SEMOP(14, 7, 1,  SEM_UNDO);  //v_producer
    SET_SEMOP(15, 8, -1, SEM_UNDO); //check_consumer
    SET_SEMOP(16, 8, 1, SEM_UNDO);
    

    int semop_ = semop(sems, INIT, 6);
    D printf("init-ed %d\n", semop_);

    semop_ = semop(sems, WRITE, 1);
    lseek(input_fd, 0, SEEK_SET);
    D printf("write-ed\n");

    semop_ = semop(sems, V_PROD, 1);
    semop_ = semop(sems, CHECK_C, 2);

    while (1) {
        read_s = read(input_fd, buff, buff_size);

        errno = 0;
        semop_ = semop(sems, ENTRY, 4);
            /*semop_ = semop(sems, P_SEM2, 1);
            D printf("sem2-ed\n");
            semop_ = semop(sems, P_MUTE, 1);
            D printf("mute-ed\n");*/
        if (semop_ == -1 && errno == EAGAIN) {
            printf("consumer has been murdered\n");
            return 0;
        }

        if (read_s > 0 && read_s == buff_size) {
            memcpy(mem_p, buff, read_s);
            D printf("copy-ed\n");

            semop_ = semop(sems, V_TRASH, 1);
            D printf("untrash-ed\n");
        }
        else if (read_s > 0 && read_s < buff_size) {
            memcpy(mem_p, buff, read_s);
            D printf("copy-ed\n");
            clear_rest(read_s, mem_p);

            semop_ = semop(sems, V_TRASH, 1);
            D printf("untrash-ed\n");
        }
        else if (read_s == 0) {
            mem_p[0] = EOF;
            clear_rest(1, mem_p);

            semop_ = semop(sems, V_TRASH, 1);
            D printf("untrash-ed\n");

            semop_ = semop(sems, V_MUTE, 1);
            semop_ = semop(sems, V_SEM1, 1);
            break;
        }
        else {
            printf("smth went wrong!\n(Do not believe consumer)\n");
            semop_ = semop(sems, V_MUTE, 1);
            semop_ = semop(sems, V_SEM1, 1);
            return 0;
        }

        semop_ = semop(sems, V_MUTE, 1);
        D printf("unmute-ed\n");
        semop_ = semop(sems, V_SEM1, 1);
        D printf("unsem1-ed\n");
    }

    D printf("exit-ed\n");

    return 0;
}

void clear_rest (int read_s, char *mem_p) {
    for ( ; read_s < buff_size; read_s++ ) {
        mem_p[read_s] = '\0';
    }
}
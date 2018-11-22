#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <errno.h>

#define SET_SEMOP(num, s_num, op, flag)     \
    semops[num].sem_num = s_num;            \
    semops[num].sem_op  = op;               \
    semops[num].sem_flg = flag;

#define INIT        semops + 0
#define P_READ      semops + 6
#define P_SEM1      semops + 9
#define P_MUTE      semops + 10
#define V_MUTE      semops + 11
#define V_SEM2      semops + 12
#define P_TRASH     semops + 13
#define CHECK_TRASH semops + 13
#define V_CONS      semops + 15
#define CHECK_P     semops + 16
#define ENTRY       semops + 7

#define D if (0)

enum semaphores {
    init,
    read,
    write,
    mutex,
    sem1,
    sem2,
    trash,
    producer,
    consumer
}

size_t buff_size = 1024;
int sem_key = 42;
int shm_key = 3802;

void freee(void* ptr) {
    free(*(void**)ptr);
}

int main(int argc, char ** argv) {
    if (argv == NULL || argc != 1) {
        printf("Ayayay!\n");
        return 0;
    }
    
    char *buff __attribute__((cleanup(freee))) 
        = calloc(buff_size, sizeof(char));
    int sems = semget(sem_key, 9, IPC_CREAT | 0644); 
    int shared_mem = shmget(shm_key, buff_size, IPC_CREAT | 0644);
    char* mem_p = shmat(shared_mem, NULL, 0);    

    struct sembuf semops[18];                                    
    SET_SEMOP(0, init, 0,   IPC_NOWAIT);//init                        
    SET_SEMOP(1, init, 1,   IPC_NOWAIT);                             
    SET_SEMOP(2, read, 1,   IPC_NOWAIT);                          
    SET_SEMOP(3, write, 1,   IPC_NOWAIT);                             
    SET_SEMOP(4, mute, 1,   IPC_NOWAIT);                               
    SET_SEMOP(5, sem2, 1,   IPC_NOWAIT);
    
    SET_SEMOP(6, read, -1,  SEM_UNDO);  //p_read  

    SET_SEMOP(7, producer, -1, SEM_UNDO | IPC_NOWAIT);  //entry(4)
    SET_SEMOP(8, producer, 1,  SEM_UNDO | IPC_NOWAIT);                
    SET_SEMOP(9, sem1, -1,  0);         //p_sem1
    SET_SEMOP(10, mute, -1,  SEM_UNDO);  //p_mute
    
    SET_SEMOP(11, mute, 1,   SEM_UNDO);  //v_mute
    
    SET_SEMOP(12, sem2, 1,  0);         //v_sem2
    
    SET_SEMOP(13, trash, -1, IPC_NOWAIT);//check_trash(2)
    
    SET_SEMOP(14, trash, 1,  IPC_NOWAIT);
    
    SET_SEMOP(15, consumer, 1, SEM_UNDO);  //v_consumer
    
    SET_SEMOP(16, producer, -1, SEM_UNDO);  //check_producer(2)
    SET_SEMOP(17, producer, 1,  SEM_UNDO);

    int semop_ = semop(sems, INIT, 6);
    D printf("init-ed %d\n", semop_);

    semop_ = semop(sems, P_READ, 1);
    D printf("read-ed\n");
    
    semop_ = semop(sems, V_CONS, 1);
    semop_ = semop(sems, CHECK_P, 2);

    while (1) {
        errno = 0;
        semop_ = semop(sems, ENTRY, 4);
        if (semop_ == -1 && errno == EAGAIN) {
            printf("producer has been murdered\n");
            return 0;
        }
            /*semop_ = semop(sems, P_SEM1, 1);
            D printf("sem1-ed\n");
            semop_ = semop(sems, P_MUTE, 1);
            D printf("mute-ed\n");*/
        
        errno = 0;
        semop_ = semop(sems, CHECK_TRASH, 2);
        D printf("checked-ed\n");
        if (semop_ == -1 && errno == EAGAIN) {
            semop_ = semop(sems, V_MUTE, 1);
            D printf("unmuted-ed\n");
            semop_ = semop(sems, V_SEM2, 1);
            D printf("unsem2-ed\n");
            continue;
        }

        if (mem_p[0] == EOF) {
            semop_ = semop(sems, P_TRASH, 1);
            D printf("trash-ed\n");
            
            semop_ = semop(sems, V_MUTE, 1);
            semop_ = semop(sems, V_SEM2, 1);

            break;
        }

        memcpy(buff, mem_p, buff_size);
        D printf("copy-ed\n");

        semop_ = semop(sems, P_TRASH, 1);
        D printf("trash-ed\n");
        
        semop_ = semop(sems, V_MUTE, 1);
        D printf("unmut-ed\n");
        semop_ = semop(sems, V_SEM2, 1);
        D printf("unsem2-ed\n");

        write(STDOUT_FILENO, buff, buff_size);
        D printf("print-ed\n");     
    }

    D printf("exit-ed\n");

    return 0;
}
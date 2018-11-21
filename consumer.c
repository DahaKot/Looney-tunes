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
#define P_FM        semops + 6
#define V_ME        semops + 8
#define P_BC        semops + 10
#define V_BC        semops + 11
#define P_BP        semops + 12
#define V_BP        semops + 13
#define READ        semops + 14
#define INCR        semops + 15
#define TEST_END    semops + 16

size_t buff_size = 1024;
int sem_key = 42;
int shm_key = 3802;

int main(int argc, char ** argv) {
    if (argv == NULL || argc != 1) {
        printf("Ayayay!\n");
        return 0;
    }
    
    char *buff = calloc(buff_size, sizeof(char));
    int sems = semget(sem_key, 9, IPC_CREAT | 0644);                //sems:
    int shared_mem = shmget(shm_key, buff_size, IPC_CREAT | 0644);  //sems[0] = mutex 
    char* mem_p = shmat(shared_mem, NULL, 0);                       //sems[1] = full

    struct sembuf semops[18];                                       //sems[2] = empty
    SET_SEMOP(0, 0, 1,   IPC_NOWAIT);                                //sems[3] = busy_consumer
    SET_SEMOP(1, 2, 1,   IPC_NOWAIT);                                //sems[4] = busy_producer
    SET_SEMOP(2, 5, 0,   IPC_NOWAIT);                                //sems[5] = init
    SET_SEMOP(3, 5, 1,   IPC_NOWAIT);                                //sems[6] = read
    SET_SEMOP(4, 6, 1,   IPC_NOWAIT);                                //sems[8] = num_processess
    SET_SEMOP(5, 7, 1,   IPC_NOWAIT);
    SET_SEMOP(6, 1, -1,  SEM_UNDO);//p_full                          //sems[7] = write
    SET_SEMOP(7, 0, -1,  SEM_UNDO);//p_mtex
    SET_SEMOP(8, 0, 1,   SEM_UNDO);//v_mutex
    SET_SEMOP(9, 2, 1,   SEM_UNDO);//v_emty
    SET_SEMOP(10, 3, -1, SEM_UNDO);//p_bc
    SET_SEMOP(11, 3, 1,  SEM_UNDO);//v_bc
    SET_SEMOP(12, 4, -1, SEM_UNDO);//p_bp
    SET_SEMOP(13, 4, 1,  SEM_UNDO);//v_bp
    SET_SEMOP(14, 6, -1, SEM_UNDO);//read
    SET_SEMOP(15, 8, 1,  SEM_UNDO);//incr
    SET_SEMOP(16, 8, -1, IPC_NOWAIT | SEM_UNDO);//test_end
    SET_SEMOP(17, 8, -1, IPC_NOWAIT | SEM_UNDO);

    int semop_ = semop(sems, INIT, 6);
    semop_ = semop(sems, READ, 1);
    semop_ = semop(sems, V_BC, 1);
    semop_ = semop(sems, P_BP, 1);
    
    while (1) {
        semop_ = semop(sems, P_FM, 2);
         
        if (mem_p[0] == EOF) {
            semop_ = semop(sems, V_ME, 2);
            break;
        }

        memcpy(buff, mem_p, buff_size);
        
        semop_ = semop(sems, V_ME, 2);

        write(STDOUT_FILENO, buff, buff_size);      
    }

    semop_ = semop(sems, V_BP, 1);
    semop_ = semop(sems, P_BC, 1);

    return 0;
}
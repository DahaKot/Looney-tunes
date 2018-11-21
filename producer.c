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
#define P_EM        semops + 6
#define V_MF        semops + 8
#define P_BC        semops + 10
#define V_BC        semops + 11
#define P_BP        semops + 12
#define V_BP        semops + 13
#define WRITE       semops + 14
#define INCR        semops + 15
#define TEST_END    semops + 16

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

    struct sembuf semops[18];
    SET_SEMOP(0, 0, 1,   IPC_NOWAIT);
    SET_SEMOP(1, 2, 1,   IPC_NOWAIT);
    SET_SEMOP(2, 5, 0,   IPC_NOWAIT);
    SET_SEMOP(3, 5, 1,   IPC_NOWAIT);
    SET_SEMOP(4, 6, 1,   IPC_NOWAIT);
    SET_SEMOP(5, 7, 1,   IPC_NOWAIT);
    SET_SEMOP(6, 2, -1,  SEM_UNDO);//p_empty
    SET_SEMOP(7, 0, -1,  SEM_UNDO);//p_mutex
    SET_SEMOP(8, 0, 1,   SEM_UNDO);//v_mutex
    SET_SEMOP(9, 1, 1,   SEM_UNDO);//v_full
    SET_SEMOP(10, 3, -1, SEM_UNDO);//p_bc
    SET_SEMOP(11, 3, 1,  SEM_UNDO);//v_bc
    SET_SEMOP(12, 4, -1, SEM_UNDO);//p_bp
    SET_SEMOP(13, 4, 1,  SEM_UNDO);//v_bp
    SET_SEMOP(14, 7, -1, SEM_UNDO);//write
    SET_SEMOP(15, 8, 1,  SEM_UNDO);//incr
    SET_SEMOP(16, 8, -1, IPC_NOWAIT | SEM_UNDO);//test_end
    SET_SEMOP(17, 8, -1, IPC_NOWAIT | SEM_UNDO);

    int semop_ = semop(sems, INIT, 6);
    semop_ = semop(sems, WRITE, 1);
    semop_ = semop(sems, V_BP, 1);
    semop_ = semop(sems, P_BC, 1);

    lseek(input_fd, 0, SEEK_SET);

    while (1) {
        read_s = read(input_fd, buff, buff_size);

        semop_ = semop(sems, P_EM, 2);

        if (read_s > 0) {
            memcpy(mem_p, buff, read_s);

            if (read_s < buff_size) {
                clear_rest(read_s, mem_p);   
            }
        }
        else {
            mem_p[0] = EOF;
            semop_ = semop(sems, V_MF, 2);
            break;
        }

        semop_ = semop(sems, V_MF, 2);
    }

    semop_ = semop(sems, V_BC, 1);
    semop_ = semop(sems, P_BP, 1);

    return 0;
}

void clear_rest (int read_s, char *mem_p) {
    for (int i = read_s; i < buff_size; i++ ) {
        mem_p[i] = '\0';
    }
}
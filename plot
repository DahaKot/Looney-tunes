//further ideas
-atomic V_MUTE + V_SEM1 and others
//work with critical section:
we have three critical section here:
1) for memory between producer and consumer
2) for semaphore that permits start of while(1) loop before consumers/producers
3) for memory between producers/consumers

all semaphore with init:
0 init = 0 + 1
1 read = 1
2 writ = 1
3 mute = 1
4 sem1 = 0
5 sem2 = 1
6 traш = 0
7 producer = 0
8 consumer = 0

//simpliest
//consumer:
get ready

p_read

while(1) {
	p_sem1, p_mutex
		//critical section
	v_mutex, v_sem2
	post_code
}
end (+undo for read)

//producer:
get ready

p_write

while(1) {
	pre_code
	p_sem2, p_mutex
		//critical section
	v_mutex, v_sem1
}
end (+undo for write)

//endings processing
//consumer:
get ready

p_read

while(1) {
	p_sem1, p_mutex
		IF TRASH:
			IGNORE
			v_mutex, v_sem2
			CONTINUE
		IF END:
			TRASH
			v_mutex, v_sem2
			BREAK

		RECIEVE & TRASH
	v_mutex, v_sem2
	WRITE_IN_STDOUT
}
end (+undo for read)

//producer:
get ready

p_write

while(1) {
	READ_FROM_FILE
	p_sem2, p_mutex
		IF READ_S > 0 AND READ_S == BUFF_SIZE:
			SEND & UNTRASH
		ELSE IF READ_S > 0 AND READ_S < BUFF_SIZE:
			SEND_WHAT_YOU_CAN
			CLEAR_REST
			UNTRASH
		ELSE IF READ_S == 0:
			WRITE EOF
			CLEAR_REST
			UNTRASH
			v_mutex, v_sem1
			BREAK
		ELSE:
			ALYARMA!!!

	v_mutex, v_sem1
}
end (+undo for write)

//murders processing
//consumer:
get ready

p_read

v_consumer 
check_producer 

while(1) {
	check_another_process_is_alive(producer) & p_sem1 & p_mutex //& - atomic cause process can be dead between check and p_sem1
		//critical_section 
	v_mutex, v_sem2
	post_code
}
end (+undo for read and consumer_alive)

check_another_process_is_alive
if not:
	die

//producer:
get ready

p_write

v_producer
check_consumer

while(1) {
	pre_code
	check_another_process_is_alive(consumer) & p_sem2 & p_mutex
		//critical_section1
	v_mutex, v_sem1
}
end (+undo for write and producer_alive)

check_another_process_is_alive
if not:
	die

tests:
c p passed
p c passed
c c p p passed
p p c c failed
c c p c p p passed
p p c p c c failed
c c c p p p passed
p p p c c c passed
//work with critical section:
pre_code
	p_sem1, p_mutex
		critical section
	v_mutex, v_sem2
post_code

we have two critical section here:
1) for memory between producer and consumer
2) for semaphore that permits start of while(1) loop before consumers/producers

init:
0 init = 0 + 1
1 read = 1
2 writ = 1
3 mute = 1
4 sem1 = 0
5 sem2 = 1
6 traш = 0

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
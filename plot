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
bc = 0
bp = 1
read = 1
write = 1
mutex = 1
full = 0
empty = 1

//consumer:
get ready

//p_bc
	p_read
//v_ bp

while(1) {
	pre_code
	p_sem1, p_mutex
		critical section
	v_mutex, v_sem2
	post_code
}
end (+undo for read)

//producer:
get ready

//p_bp
	p_write
//v_bc

while(1) {
	pre_code
	p_sem2, p_mutex
		critical section
	v_mutex, v_sem1
	post_code
}
end (+undo for write)
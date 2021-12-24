# ZeOS-SOA 2021-2022 - Proyecto Pipes
### Project mark: `7`

En esta rama se encuentra el proyecto de SOA. Básicamente era implementar pipes. A continuación el enunciado:

## Description of the project
Mr. Baka Baka, as a visionary of the Operating System market, has realized that ZeOS has a lot of
potential to compete with other commercial operating systems.

For that, he decides to start implementing features not currently present in ZeOS. One of these features
is pipes.

A pipe is an inter-process communication mechanism that, by means of a shared buffer, allows the
communication of data between related (parent-child, brother-brother, etc) processes.

To create a pipe, use the system call:
`int pipe(int *pd);`

in which pd is declared as int pd[2] and it is where the system call will return the file descriptors
corresponding to the read channel (pd[0]) and write channel (pd[1]) of the pipe. Everything that is
written in pd[1] will be available for reading in pd[0]. If everything is ok, the pipe system call returns 0.
Otherwise, it returns -1 setting a value describing the error in errno.

After a pipe is created, read and write system calls can be executed using the returned file descriptors.
In this case, read is a new system call and write changes its behavior:

- The read system call:
`int read(int fd, void *buf, size_t count);`

read count bytes from file descriptor fd, storing them at memory region buf. Reading from an empty
pipe blocks the process until:

- there is data available in the pipe, or
- all the write channels of the pipe are closed. In this case, read returns 0.
- Writing to a filled pipe, blocks the process until:
- there is room in the pipe for writing more data, or
- all the read channels of the pipe are closed. In this case, write returns -1, setting errno to
EPIPE.

Notice that for closing a pipe, all the read and write channels must be closed. 

# Milestones
- [x] 1. (1 point) Implement kernel semaphores in ZeOS.
- [x] 2. (1 point) Implement the file system structures (table of channels and opened file table).
- [x] 3. (1 point) Implement the pipe system call (wrapper and service routine).
- [x] 4. (1 point) Implement the read system call (wrapper and service routine).
- [x] 5. (1 point) Modify the write system call to work with pipes.
- [x] 6. (1 point) Modify the sys_fork service routine to inherit the pipes of the process.
- [x] 7. (1 point) Implement the close system call (wrapper and service routine).
- [x] 8. (1 point) Modify the sys_exit service routine to work with pipes.
- [ ] 9. (1 point) Implement workloads to measure the transmission bandwidth of pipes in ZeOS
- [ ] 10. (1 point) Implement an optimization to increase the transmission bandwidth of pipes (comment
the optimization you plan to implement with the teacher, beforehand) 

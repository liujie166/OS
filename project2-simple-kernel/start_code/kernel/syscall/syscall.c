#include "lock.h"
#include "sched.h"
#include "common.h"
#include "screen.h"
#include "syscall.h"

void system_call_helper(int fn, int arg1, int arg2, int arg3)
{

    (*syscall[fn])(arg1, arg2, arg3);

}

void sys_sleep(uint32_t time)
{
    invoke_syscall(SYSCALL_SLEEP, time, IGNORE, IGNORE);
}

void sys_block(queue_t *queue)
{
    invoke_syscall(SYSCALL_BLOCK, (int)queue, IGNORE, IGNORE);
}

void sys_unblock_one(queue_t *queue)
{
    invoke_syscall(SYSCALL_UNBLOCK_ONE, (int)queue, IGNORE, IGNORE);
}

void sys_unblock_all(queue_t *queue)
{
    invoke_syscall(SYSCALL_UNBLOCK_ALL, (int)queue, IGNORE, IGNORE);
}

void sys_write(char *buff)
{
    invoke_syscall(SYSCALL_WRITE, (int)buff, IGNORE, IGNORE);
}

void sys_reflush()
{
    invoke_syscall(SYSCALL_REFLUSH, IGNORE, IGNORE, IGNORE);
}

void sys_move_cursor(int x, int y)
{
    invoke_syscall(SYSCALL_CURSOR, x, y, IGNORE);
}

void mutex_lock_init(mutex_lock_t *lock)
{
    invoke_syscall(SYSCALL_MUTEX_LOCK_INIT, (int)lock, IGNORE, IGNORE);
}

void mutex_lock_acquire(mutex_lock_t *lock)
{
    invoke_syscall(SYSCALL_MUTEX_LOCK_ACQUIRE, (int)lock, IGNORE, IGNORE);
}

void mutex_lock_release(mutex_lock_t *lock)
{
    invoke_syscall(SYSCALL_MUTEX_LOCK_RELEASE, (int)lock, IGNORE, IGNORE);
}

void barrier_init(barrier_t * barrier, int num_task)
{
    invoke_syscall(SYSCALL_BARRIER_INIT, (int)barrier, num_task, IGNORE);
}

void barrier_wait(barrier_t * barrier)
{
    invoke_syscall(SYSCALL_BARRIER_WAIT, (int)barrier, IGNORE, IGNORE);
}

void condition_init(condition_t * condition)
{
    invoke_syscall(SYSCALL_CONDITION_INIT, (int)condition, IGNORE, IGNORE);
}

void condition_signal(condition_t * condition)
{
    invoke_syscall(SYSCALL_CONDITION_SIGNAL, (int)condition, IGNORE, IGNORE);
}

void condition_broadcast(condition_t * condition)
{
    invoke_syscall(SYSCALL_CONDITION_BROADCAST, (int)condition, IGNORE, IGNORE);
}

void condition_wait(mutex_lock_t * lock, condition_t * condition)
{
    invoke_syscall(SYSCALL_CONDITION_WAIT, (int)lock, (int)condition, IGNORE);
}

void sys_exit()
{
    invoke_syscall(SYSCALL_EXIT, IGNORE, IGNORE, IGNORE);
}

void sys_waitpid(pid_t pid)
{
    invoke_syscall(SYSCALL_WAITPID, (int)pid, IGNORE, IGNORE);
}

void sys_spawn(task_info_t * task)
{
    invoke_syscall(SYSCALL_SPAWN, (int)task, IGNORE, IGNORE);
}

void sys_kill(pid_t pid)
{
    invoke_syscall(SYSCALL_KILL, (int)pid, IGNORE, IGNORE);
}

void sys_getpid(pid_t* mypid)
{
    invoke_syscall(SYSCALL_GETPID, (int)mypid, IGNORE, IGNORE);
}

void semaphore_init(semaphore_t * semaphore, int value)
{
    invoke_syscall(SYSCALL_SEMAPHORE_INIT, (int)semaphore, value, IGNORE);
}

void semaphore_up(semaphore_t * semaphore)
{
    invoke_syscall(SYSCALL_SEMAPHORE_UP, (int)semaphore, IGNORE, IGNORE);
}

void semaphore_down(semaphore_t * semaphore)
{
    invoke_syscall(SYSCALL_SEMAPHORE_DOWN, (int)semaphore, IGNORE, IGNORE);
}
void sys_ps()
{
     invoke_syscall(SYSCALL_PS, IGNORE, IGNORE, IGNORE);
}

void sys_clear()
{
    invoke_syscall(SYSCALL_CLEAR, IGNORE, IGNORE, IGNORE);
}

void sys_wait(pid_t pid)
{
    invoke_syscall(SYSCALL_WAIT,(int) pid,IGNORE,IGNORE);
}
void sys_init_mac(void)
{
    invoke_syscall(SYSCALL_INIT_MAC, IGNORE, IGNORE, IGNORE);
}
void sys_net_send(uint32_t td, uint32_t td_phy)
{
    invoke_syscall(SYSCALL_NET_SEND, (int)td, (int)td_phy, IGNORE);
}
uint32_t sys_net_recv(uint32_t rd, uint32_t rd_phy, uint32_t daddr)
{
    return invoke_syscall(SYSCALL_NET_RECV,(int)rd,(int)rd_phy,(int)daddr);
}
void sys_wait_recv_package()
{
    invoke_syscall(SYSCALL_WAIT_RECV_PACKAGE, IGNORE, IGNORE, IGNORE);
}
void sys_init_fs()
{
    invoke_syscall(SYSCALL_INIT_FS, IGNORE, IGNORE, IGNORE);
}
void sys_fs_state()
{
    invoke_syscall(SYSCALL_FS_STATE, IGNORE, IGNORE, IGNORE);
}
void sys_mkdir(char* name)
{
    invoke_syscall(SYSCALL_MKDIR, (int) name, IGNORE, IGNORE);
}
void sys_ls()
{
    invoke_syscall(SYSCALL_LS, IGNORE, IGNORE, IGNORE);
}
void sys_rmdir(char* name)
{
    invoke_syscall(SYSCALL_RMDIR, (int) name, IGNORE, IGNORE);
}
void sys_cd(char* path)
{
    invoke_syscall(SYSCALL_CD, (int) path, IGNORE, IGNORE);
}
void sys_touch(char* name){
    invoke_syscall(SYSCALL_TOUCH, (int) name, IGNORE, IGNORE);
}
void sys_cat(char* name){
    invoke_syscall(SYSCALL_CAT, (int) name, IGNORE, IGNORE);   
}
int sys_fopen(char *name,int access){
    invoke_syscall(SYSCALL_FOPEN, (int) name, access, IGNORE);   
}

int sys_fread(int fd,char *buffer,int size){
    invoke_syscall(SYSCALL_FREAD, (int) fd, (int) buffer, size);      
}
int sys_fwrite(int fd,char *buffer,int size){
    invoke_syscall(SYSCALL_FWRITE, (int) fd, (int) buffer, size);   
}
void sys_fclose(int fd){
    invoke_syscall(SYSCALL_FCLOSE, (int) fd, IGNORE, IGNORE);   
}
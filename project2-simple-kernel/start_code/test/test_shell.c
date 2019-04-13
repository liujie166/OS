/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *            Copyright (C) 2018 Institute of Computing Technology, CAS
 *               Author : Han Shukai (email : hanshukai@ict.ac.cn)
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 *                  The shell acts as a task running in user mode. 
 *       The main function is to make system calls through the user's output.
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * *
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy of this 
 * software and associated documentation files (the "Software"), to deal in the Software 
 * without restriction, including without limitation the rights to use, copy, modify, 
 * merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit 
 * persons to whom the Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 * 
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *  * * * * * * * * * * */

#include "test.h"
#include "stdio.h"
#include "screen.h"
#include "syscall.h"
#include "string.h"
#include "sched.h"
#include "fs.h"
// pid = 2
// test exit
static char buff[64];

void test_fs(void)
{
    int i, j;
    int fd = sys_fopen("1.txt", 3);

    for (i = 0; i < 10; i++)
    {
        sys_fwrite(fd, "hello world!\n", 13);
    }
    sys_move_cursor(0,2);
    for (i = 0; i < 10; i++)
    {
        sys_fread(fd, buff, 13);

        for (j = 0; j < 13; j++)
        {
            printf("%c", buff[j]);
        }
    }

    sys_fclose(fd);
    sys_exit();
}

void test_shell_task1()
{
    int i;

    for (i = 0; i < 500; i++)
    {
        sys_move_cursor(0, 0);
        printf("I am Task A.(%d)           \n", i);
    }

    sys_exit();
}

// pid = 3
// test kill & waitpid
void test_shell_task2()
{
    int i;

    sys_move_cursor(0, 1);
    printf("I am waiting Task A to exit.\n");
    sys_waitpid(2);

    for (i = 0;; i++)
    {
        sys_move_cursor(0, 1);
        printf("I am Task B.(%d)           \n", i);
    }
}

// pid = 4
// test waitpid
void test_shell_task3()
{
    int i;

    sys_move_cursor(0, 2);
    printf("I am waiting Task B to exit.\n");
    sys_waitpid(3);

    for (i = 0;; i++)
    {
        sys_move_cursor(0, 2);
        printf("I am Task C.(%d)           \n", i);
    }
}



#define SHLLE_BUFF_SIZE 64

static int shell_tail = 0;
static char shell_buff[SHLLE_BUFF_SIZE];

static char read_uart_ch(void)
{
    char ch = 0;
    unsigned char *read_port = (unsigned char *)(0xbfe48000 + 0x00);
    unsigned char *stat_port = (unsigned char *)(0xbfe48000 + 0x05);

    while ((*stat_port & 0x01))
    {
        ch = *read_port;
    }
    return ch;
}

struct task_info task1 = {"task1", (uint32_t)&ready_to_exit_task, USER_PROCESS};
struct task_info task2 = {"task2", (uint32_t)&wait_lock_task, USER_PROCESS};
struct task_info task3 = {"task3", (uint32_t)&wait_exit_task, USER_PROCESS};

struct task_info task4 = {"task4", (uint32_t)&semaphore_add_task1, USER_PROCESS};
struct task_info task5 = {"task5", (uint32_t)&semaphore_add_task2, USER_PROCESS};
struct task_info task6 = {"task6", (uint32_t)&semaphore_add_task3, USER_PROCESS};

struct task_info task7 = {"task7", (uint32_t)&producer_task, USER_PROCESS};
struct task_info task8 = {"task8", (uint32_t)&consumer_task1, USER_PROCESS};
struct task_info task9 = {"task9", (uint32_t)&consumer_task2, USER_PROCESS};

struct task_info task10 = {"task10", (uint32_t)&barrier_task1, USER_PROCESS};
struct task_info task11 = {"task11", (uint32_t)&barrier_task2, USER_PROCESS};
struct task_info task12 = {"task12", (uint32_t)&barrier_task3, USER_PROCESS};

struct task_info task13 = {"SunQuan",(uint32_t)&SunQuan, USER_PROCESS};
struct task_info task14 = {"LiuBei", (uint32_t)&LiuBei, USER_PROCESS};
struct task_info task15 = {"CaoCao", (uint32_t)&CaoCao, USER_PROCESS};
#if 1
struct task_info task4_1 = {"send",(uint32_t)&phy_regs_task1, USER_PROCESS};
struct task_info task4_2 = {"recv",(uint32_t)&phy_regs_task2, USER_PROCESS};
struct task_info task4_3 = {"initmac",(uint32_t)&phy_regs_task3, USER_PROCESS};


static struct task_info *test_tasks[19] = {&task4_3,&task4_1,&task4_2};
static int num_test_tasks = 3;
#endif


struct task_info test = {"test_fs", (uint32_t)&test_fs, USER_PROCESS};

void decode_inst(char * str, int str_len)
{
    char name[15];
    char inst[15];
    int i,j;
    int* pid;
    for(i=0,j=0;i<str_len;i++,j++){
        if(str[i]=='\b'){
            inst[j-1] = '\0';
            j = j-2;
        }
        else
            inst[j] =  str[i];
    }
    inst[j] = '\0';

    if(inst[0] == 'p'&&inst[1] == 's')
    {

        sys_ps();
        printf("[RUNNING PROCESS]\n");
        for(i=0;running_pid[i]!=-1&&i<16;i++){
            printf("[%d] PID : %d STATUS : RUNNING\n",i,running_pid[i]);
        }
        return;
    }
    else if(inst[0] == 'c'&&inst[1] == 'l'&&inst[2] == 'e'&&inst[3] == 'a'&&inst[4]=='r')
    {
        sys_clear();
        sys_move_cursor(0,15);
        printf("------------------- COMMAND -------------------\n");
        print_loc = 17;
        return;
    }
    else if(inst[0] == 'e'&&inst[1] == 'x'&&inst[2] == 'e'&&inst[3] == 'c'&&inst[4]==' '&&inst[5]!='\0')
    {
        if(inst[7]=='\0'){
            sys_spawn(test_tasks[(inst[5]-'0')*10+inst[6]-'0']);
            printf("process[%d] begin to run\n", (inst[5]-'0')*10+inst[6]-'0');
        }
        else if(inst[6]=='\0'){
            sys_spawn(test_tasks[inst[5]-'0']);
            printf("process[%d] begin to run\n", inst[5]-'0');
        }
        else
            printf("error goal process\n");

        return;
    }
    else if(inst[0] == 'k'&&inst[1] == 'i'&&inst[2] == 'l'&&inst[3] == 'l'&&inst[4]==' '&&inst[5]!='\0')
    {
        if(inst[7]=='\0'){
            sys_kill((inst[5]-'0')*10+inst[6]-'0');
            printf("process[%d] is killed\n",(inst[5]-'0')*10+inst[6]-'0');
        }
        else if(inst[6]=='\0'){
            sys_kill(inst[5]-'0');
            printf("process[%d] is killed\n",inst[5]-'0');
        }
        else
            printf("error goal process\n");
        return;
    }
    else if(inst[0] == 'm'&&inst[1] == 'k'&&inst[2] == 'f'&&inst[3] == 's'){
        sys_init_fs();

    }
    else if(inst[0] == 's'&&inst[1] == 't'&&inst[2] == 'a'&&inst[3] == 't'&&inst[4]=='f'&&inst[5]=='s'){
        sys_fs_state();

    }
    else if(inst[0] == 'm'&&inst[1] == 'k'&&inst[2] == 'd'&&inst[3] == 'i'&&inst[4]=='r'&&inst[5]==' '){
        strcpy(name,&inst[6]);
        sys_mkdir(name);
        printf("\n");
        print_loc++;
    }
    else if(inst[0] == 'r'&&inst[1] == 'm'&&inst[2] == 'd'&&inst[3] == 'i'&&inst[4]=='r'&&inst[5]==' '){
        strcpy(name,&inst[6]);
        sys_rmdir(name);
        printf("\n");
        print_loc++;
    }
    else if(inst[0] == 'l'&&inst[1] == 's'){
        sys_ls();
        printf("\n");
        print_loc++;
    }
    else if(inst[0]== 'c'&&inst[1] == 'd'&&inst[2] == ' '){
        strcpy(name,&inst[3]);
        sys_cd(name);
        printf("\n");
        print_loc++;
    }
    else if(inst[0]== 'c'&&inst[1] == 'a'&&inst[2] == 't'&&inst[3]==' '){
        strcpy(name,&inst[4]);
        sys_cat(name);
        printf("\n");
        print_loc++;
    }
    else if(inst[0] == 't'&&inst[1] == 'o'&&inst[2] == 'u'&&inst[3] == 'c'&&inst[4]=='h'&&inst[5]==' '){
        strcpy(name,&inst[6]);
        sys_touch(name);
        printf("\n");
        print_loc++;
    }
    else if(inst[0]== 't'&&inst[1] == 'e'&&inst[2] == 's'&&inst[3]=='t'){
        sys_spawn(&test);
    }
    else
        printf("error inst\n");
    return;
}
void test_shell()
{
    int print_locatation = 15;
    int i=0;
    int str_len=0;
    char str[20];
    sys_move_cursor(0,print_locatation);
    printf("------------------- COMMAND -------------------\n");
    printf("root@MY_OS: ");
    print_loc = 18;
    while (1)
    {
        // read command from UART port
        //mutex_lock_acquire(&screen_lock);
        disable_interrupt();
        char ch = read_uart_ch();
        enable_interrupt();
        //mutex_lock_release(&screen_lock);
        // TODO solve command
        if(ch!=0) {
            if(ch!='\r'){
                str[i++] = ch;
                str[i] = '\0';
                str_len++;
                printf("%c",ch);
            }
            if(ch=='\r'){
                printf("\n");
                decode_inst(str, str_len);
                print_loc++;
                printf("root@MY_OS: %s$  ",current_path);
                i=0;
                str_len=0;
            }
        }
    }   
}

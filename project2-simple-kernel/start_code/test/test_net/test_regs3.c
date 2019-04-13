#include "mac.h"
#include "irq.h"
#include "type.h"
#include "screen.h"
#include "syscall.h"
#include "sched.h"
#include "test4.h"


desc_t *send_desc;
desc_t *receive_desc;
int login_flag =0;
uint32_t cnt =0; //record the time of iqr_mac
//uint32_t buffer[PSIZE] = {0x00040045, 0x00000100, 0x5d911120, 0x0101a8c0, 0xfb0000e0, 0xe914e914, 0x00000801,0x45000400, 0x00010000, 0x2011915d, 0xc0a80101, 0xe00000fb, 0x14e914e9, 0x01080000};
uint32_t buffer[PSIZE] = {0xffffffff, 0x5500ffff, 0xf77db57d, 0x00450008, 0x0000d400, 0x11ff0040, 0xa8c073d8, 0x00e00101, 0xe914fb00, 0x0004e914, 0x0000, 0x005e0001, 0x2300fb00, 0x84b7f28b, 0x00450008, 0x0000d400, 0x11ff0040, 0xa8c073d8, 0x00e00101, 0xe914fb00, 0x0801e914, 0x0000};
uint32_t recv_buffer[PNUM][PNUM];
uint32_t send_buffer[PNUM];
desc_t s_desc[PNUM];
desc_t r_desc[PNUM];
/**
 * Clears all the pending interrupts.
 * If the Dma status register is read then all the interrupts gets cleared
 * @param[in] pointer to synopGMACdevice.
 * \return returns void.
 */
void clear_interrupt()
{
    uint32_t data;
    data = reg_read_32(0xbfe11000 + DmaStatus);
    reg_write_32(0xbfe11000 + DmaStatus, data);
}

static void send_desc_init(mac_t *mac)
{
    int i;
    uint32_t get_phy = (uint32_t)0x1fffffff;
    //printf("begin to init send \n");
    for(i=0;i<PNUM;i++){
        s_desc[i].tdes0 = 0x00000000;
        s_desc[i].tdes1 = 0x61000100;
        s_desc[i].tdes2 = get_phy&(uint32_t)&buffer[0];
        s_desc[i].tdes3 = get_phy&(uint32_t)&s_desc[i+1];
        if(i==(PNUM-1)){
            s_desc[i].tdes1 = 0x63000100;
            s_desc[i].tdes3 = get_phy&(uint32_t)(&s_desc[0]);
        }
    }
    mac->td = (uint32_t)&s_desc[0];
    mac->td_phy = get_phy&(uint32_t)(&s_desc[0]);
}

static void recv_desc_init(mac_t *mac)
{
    int i;
    uint32_t get_phy = (uint32_t)0x1fffffff;
    for(i=0;i<PNUM;i++){
        r_desc[i].tdes0 = 0x00000000;
        r_desc[i].tdes1 = 0x81000100;
        r_desc[i].tdes2 = get_phy&(uint32_t)&recv_buffer[i][0];
        r_desc[i].tdes3 = get_phy&(uint32_t)&r_desc[i+1];
        if(i==(PNUM-1)){
            r_desc[i].tdes1 = 0x83000100;
            r_desc[i].tdes3 = get_phy&(uint32_t)&r_desc[0];
        }
    }
    mac->rd = (uint32_t)&r_desc[0];
    mac->rd_phy = get_phy&(uint32_t)(&r_desc[0]);
}


static void mii_dul_force(mac_t *mac)
{
    reg_write_32(mac->dma_addr, 0x80); //?s
                                       //   reg_write_32(mac->dma_addr, 0x400);
    uint32_t conf = 0xc800;            //0x0080cc00;

    // loopback, 100M
    reg_write_32(mac->mac_addr, reg_read_32(mac->mac_addr) | (conf) | (1 << 8));
    //enable recieve all
    reg_write_32(mac->mac_addr + 0x4, reg_read_32(mac->mac_addr + 0x4) | 0x80000001);
}

static void start_tran(mac_t *mac)
{

   
}

void dma_control_init(mac_t *mac, uint32_t init_value)
{
    reg_write_32(mac->dma_addr + DmaControl, init_value);
    return;
}
void irq_handler(){
    while(1){
        sys_move_cursor(1, 2);
        if(cnt<64){
            printf("> [RECV TASK] still waiting %dth package.\n",cnt+1);

        }
        if(cnt==64){
            printf("> [RECV TASK] succeed to recive 64 package.\n");
            login_flag = 0;
            sys_exit();
            
        }
        if(0x00000000==(r_desc[cnt].tdes0&0x80000000)&&cnt<64){
            cnt++;
        }
        
    }
}
void register_irq_handler(irq_mac)
{
    task_info_t irq_task = {"irq", (uint32_t)&irq_handler, USER_PROCESS};
    if(login_flag == 0){
        sys_spawn(&irq_task);
        login_flag =1;
    }
}
void phy_regs_task1()
{

    mac_t test_mac;
    uint32_t i;
    uint32_t print_location = 1;
    uint32_t send_cnt;
    
    test_mac.mac_addr = 0xbfe10000;
    test_mac.dma_addr = 0xbfe11000;

    test_mac.psize = PSIZE * 4; // 64bytes
    test_mac.pnum = PNUM;       // pnum

    send_desc_init(&test_mac);
    //printf("begin to init send \n");
    dma_control_init(&test_mac, DmaStoreAndForward | DmaTxSecondFrame | DmaRxThreshCtrl128);
    clear_interrupt();

    mii_dul_force(&test_mac);

    sys_move_cursor(1, print_location);
    printf("> [SEND TASK] start send package.               \n");
    send_cnt = 0;
    i = 1;
    while (i > 0)
    {
        sys_net_send(test_mac.td, test_mac.td_phy);
        send_cnt += PNUM;
        sys_move_cursor(1, print_location);
        printf("> [SEND TASK] totally send package %d !        \n", send_cnt);
        i--;
    }
    sys_exit();
}

void phy_regs_task2()
{

    mac_t test_mac;

    uint32_t i;
    uint32_t ret;
    uint32_t print_location = 1;

    uint32_t cnt = 0;
    uint32_t *Recv_desc;
    test_mac.mac_addr = 0xbfe10000;
    test_mac.dma_addr = 0xbfe11000;

    test_mac.psize = PSIZE * 4; // 64bytes
    test_mac.pnum = PNUM;       // pnum
    recv_desc_init(&test_mac);

    dma_control_init(&test_mac, DmaStoreAndForward | DmaTxSecondFrame | DmaRxThreshCtrl128);
    clear_interrupt();

    mii_dul_force(&test_mac);
    queue_init(&recv_block_queue);
    register_irq_handler(irq_mac);

    irq_enable();
    
    sys_move_cursor(1, print_location);
    printf("[RECV TASK] start recv:                    ");
    ret = sys_net_recv(test_mac.rd, test_mac.rd_phy, test_mac.daddr);
  


    Recv_desc = (uint32_t *)(test_mac.rd + (PNUM - 1) * 16);
    //printf("(test_mac.rd 0x%x ,Recv_desc=0x%x,REDS0 0X%x\n", test_mac.rd, Recv_desc, *(Recv_desc));
    if (((*Recv_desc) & 0x80000000) == 0x80000000)
    {
        sys_move_cursor(1, print_location);
        printf("> [RECV TASK] waiting receive package.\n");
        sys_wait_recv_package();
    }

    check_recv(&test_mac);

    sys_exit();
}

void phy_regs_task3()
{
    uint32_t print_location = 1;
    sys_move_cursor(1, print_location);
    printf("> [INIT] Waiting for MAC initialization .\n");
    sys_init_mac();
    sys_move_cursor(1, print_location);
    printf("> [INIT] MAC initialization succeeded.           \n");
    sys_exit();
}

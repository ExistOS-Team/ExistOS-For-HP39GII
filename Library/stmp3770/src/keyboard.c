/*
 * A simple user interface for this project
 *
 * Copyright 2020 Creep_er
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307 USA
 */
#include "keyboard.h"
#include "regsclkctrl.h"
#include "regslcdif.h"
#include "regspinctrl.h"
#include "utils.h"

#include "irq.h"
#include "hw_irq.h"

#include "FreeRTOS.h"
#include "queue.h"
#include "task.h"

//SSP1_SCK		BANK1_PIN23
//SSP1_DATA1	BANK1_PIN25
//SSP1_DATA3	BANK1_PIN27
//SSP1_DATA2	BANK1_PIN26
//SSP1_CMD		BANK1_PIN22

unsigned char key_matrix[5][11] = {0};

unsigned char key_matrix_last_state[5][11] = {0};

void deassert_all_row_line(){
    volatile unsigned int tmp_DOUT, tmp_DOE;
    tmp_DOUT = BF_RD(PINCTRL_DOUT2, DOUT);
    tmp_DOE = BF_RD(PINCTRL_DOE2, DOE);
    tmp_DOUT &= ~((1 << 14) | (1 << 8) | (1 << 7) | (1 << 6) | (1 << 5) | (1 << 4) | (1 << 3) | (1 << 2));
    tmp_DOE |= ((1 << 14) | (1 << 8) | (1 << 7) | (1 << 6) | (1 << 5) | (1 << 4) | (1 << 3) | (1 << 2));
    BF_CS1(PINCTRL_DOUT2, DOUT, tmp_DOUT);
    BF_CS1(PINCTRL_DOE2, DOE, tmp_DOE);
    BF_CS1(PINCTRL_DOUT2, DOUT, tmp_DOUT);

    tmp_DOUT = BF_RD(PINCTRL_DOUT1, DOUT);
    tmp_DOE = BF_RD(PINCTRL_DOE1, DOE);
    tmp_DOUT &= ~((1 << 24));
    tmp_DOE |= ((1 << 24));
    BF_CS1(PINCTRL_DOUT1, DOUT, tmp_DOUT);
    BF_CS1(PINCTRL_DOE1, DOE, tmp_DOE);
    BF_CS1(PINCTRL_DOUT1, DOUT, tmp_DOUT);

    tmp_DOUT = BF_RD(PINCTRL_DOUT0, DOUT);
    tmp_DOE = BF_RD(PINCTRL_DOE0, DOE);
    tmp_DOUT &= ~((1 << 20));
    tmp_DOE |= ((1 << 20));
    BF_CS1(PINCTRL_DOUT0, DOUT, tmp_DOUT);
    BF_CS1(PINCTRL_DOE0, DOE, tmp_DOE);
    BF_CS1(PINCTRL_DOUT0, DOUT, tmp_DOUT);
}

void set_row_line(int row_line) {
    unsigned int tmp_DOUT, tmp_DOE;

    tmp_DOUT = BF_RD(PINCTRL_DOUT2, DOUT);
    tmp_DOE = BF_RD(PINCTRL_DOE2, DOE);
    tmp_DOUT |= ((1 << 14) | (1 << 8) | (1 << 7) | (1 << 6) | (1 << 5) | (1 << 4) | (1 << 3) | (1 << 2));
    tmp_DOE |= ((1 << 14) | (1 << 8) | (1 << 7) | (1 << 6) | (1 << 5) | (1 << 4) | (1 << 3) | (1 << 2));
    BF_CS1(PINCTRL_DOUT2, DOUT, tmp_DOUT);
    BF_CS1(PINCTRL_DOE2, DOE, tmp_DOE);
    BF_CS1(PINCTRL_DOUT2, DOUT, tmp_DOUT);

    tmp_DOUT = BF_RD(PINCTRL_DOUT1, DOUT);
    tmp_DOE = BF_RD(PINCTRL_DOE1, DOE);
    tmp_DOUT |= ((1 << 24));
    tmp_DOE |= ((1 << 24));
    BF_CS1(PINCTRL_DOUT1, DOUT, tmp_DOUT);
    BF_CS1(PINCTRL_DOE1, DOE, tmp_DOE);
    BF_CS1(PINCTRL_DOUT1, DOUT, tmp_DOUT);

    tmp_DOUT = BF_RD(PINCTRL_DOUT0, DOUT);
    tmp_DOE = BF_RD(PINCTRL_DOE0, DOE);
    tmp_DOUT |= ((1 << 20));
    tmp_DOE |= ((1 << 20));
    BF_CS1(PINCTRL_DOUT0, DOUT, tmp_DOUT);
    BF_CS1(PINCTRL_DOE0, DOE, tmp_DOE);
    BF_CS1(PINCTRL_DOUT0, DOUT, tmp_DOUT);

    switch (row_line) {
    case 5:
        tmp_DOUT = BF_RD(PINCTRL_DOUT1, DOUT);
        tmp_DOE = BF_RD(PINCTRL_DOE1, DOE);
        tmp_DOUT &= ~((1 << 24));
        tmp_DOE |= ((1 << 24));
        BF_CS1(PINCTRL_DOUT1, DOUT, tmp_DOUT);
        BF_CS1(PINCTRL_DOE1, DOE, tmp_DOE);
        BF_CS1(PINCTRL_DOUT1, DOUT, tmp_DOUT);
        break;
    case 8:
        tmp_DOUT = BF_RD(PINCTRL_DOUT0, DOUT);
        tmp_DOE = BF_RD(PINCTRL_DOE0, DOE);
        tmp_DOUT &= ~((1 << 20));
        tmp_DOE |= ((1 << 20));
        BF_CS1(PINCTRL_DOUT0, DOUT, tmp_DOUT);
        BF_CS1(PINCTRL_DOE0, DOE, tmp_DOE);
        BF_CS1(PINCTRL_DOUT0, DOUT, tmp_DOUT);
        break;
    default:
        tmp_DOUT = BF_RD(PINCTRL_DOUT2, DOUT);
        tmp_DOE = BF_RD(PINCTRL_DOE2, DOE);
        switch (row_line) {
        case 0:
            tmp_DOUT &= ~((1 << 6));
            break;
        case 1:
            tmp_DOUT &= ~((1 << 5));
            break;
        case 2:
            tmp_DOUT &= ~((1 << 4));
            break;
        case 3:
            tmp_DOUT &= ~((1 << 2));
            break;
        case 4:
            tmp_DOUT &= ~((1 << 3));
            break;
        case 6:
            tmp_DOUT &= ~((1 << 8));
            break;
        case 7:
            tmp_DOUT &= ~((1 << 7));
            break;
        case 9:
            tmp_DOUT &= ~((1 << 14));
            break;
        }
        tmp_DOE |= ((1 << 14) | (1 << 8) | (1 << 7) | (1 << 6) | (1 << 5) | (1 << 4) | (1 << 3) | (1 << 2));
        BF_CS1(PINCTRL_DOUT2, DOUT, tmp_DOUT);
        BF_CS1(PINCTRL_DOE2, DOE, tmp_DOE);
        BF_CS1(PINCTRL_DOUT2, DOUT, tmp_DOUT);
        break;
    }
}

unsigned int read_col_line(int col_line) {
    switch (col_line) {
    case 0:
        return (BF_RD(PINCTRL_DIN1, DIN) >> 23) & 1;
    case 1:
        return (BF_RD(PINCTRL_DIN1, DIN) >> 25) & 1;
    case 2:
        return (BF_RD(PINCTRL_DIN1, DIN) >> 27) & 1;
    case 3:
        return (BF_RD(PINCTRL_DIN1, DIN) >> 26) & 1;
    case 4:
        return (BF_RD(PINCTRL_DIN1, DIN) >> 22) & 1;
    default:
        break;
    }
    return 0;
}

extern QueueHandle_t key_msg_queue;

unsigned int act_x, act_y, key_status;
unsigned int key_msg;

void pinctrl1_bank1_isr(){
    volatile unsigned int tmp_tog, wline, rd_col_line;
    wline = BF_RD(PINCTRL_IRQSTAT1,IRQSTAT);
    delay_us(1000);
    if(wline != BF_RD(PINCTRL_IRQSTAT1,IRQSTAT)){
        BF_CLR(PINCTRL_IRQSTAT1,IRQSTAT);
        return ;
    }
    
    for(int i = 0; i< 10; i++){
        set_row_line(i);
        for(int j = 0; j<5; j++){
            rd_col_line = read_col_line(j);
            if(rd_col_line != !key_matrix_last_state[j][i])
            {
                key_matrix_last_state[j][i] = !rd_col_line;
                act_x = i;
                act_y = j;
                //uartdbg_printf("row:%x col:%x ",i,j);
            }
        }
    }
    deassert_all_row_line();
    BF_CLR(PINCTRL_IRQSTAT1,IRQSTAT); 

    tmp_tog = BF_RD(PINCTRL_IRQPOL1,IRQPOL);
    tmp_tog ^=wline;
    BF_CS1(PINCTRL_IRQPOL1,IRQPOL,tmp_tog);

    //uartdbg_printf("key irq:%x ",wline);
    if(tmp_tog & wline){
        key_status = 1;
        key_matrix[act_x][act_y] = 1;
    }else{
        key_status = 0;
        key_matrix[act_x][act_y] = 0;
    }

    if(key_msg_queue != NULL){
        key_msg = (act_x << 11) | (act_y << 8) | key_status;
        xQueueSendFromISR(key_msg_queue, &key_msg, NULL);
    }
    
    //xQueueSendFromISR()
    //deassert_all_row_line();

    //uartdbg_printf("x:%x y:%x s:%x\n",act_x, act_y, status);
    //BF_CS1(PINCTRL_IRQPOL1,IRQPOL, ((1 << 22) | (1 << 23) | (1 << 25) | (1 << 26) | (1 << 27))  );
}

void pinctrl0_key_on_isr(){
    volatile unsigned int status, tmp_tog;
    status = BF_RD(PINCTRL_IRQSTAT0, IRQSTAT);
    delay_us(1000);
    if(status != BF_RD(PINCTRL_IRQSTAT0,IRQSTAT)){
        BF_CLR(PINCTRL_IRQSTAT0,IRQSTAT);
        return ;
    }
    BF_CLR(PINCTRL_IRQSTAT0,IRQSTAT); 


    tmp_tog = BF_RD(PINCTRL_IRQPOL0,IRQPOL);
    tmp_tog ^=status;
    BF_CS1(PINCTRL_IRQPOL0,IRQPOL,tmp_tog);
    if(tmp_tog & status){
        //POP ON
        key_msg = KEY_ON << 8 | 0;
        key_matrix[0][10] = 1;
        if(key_msg_queue != NULL)
            xQueueSendFromISR(key_msg_queue, &key_msg, NULL);

    }else{  
        //PUSH ON
        key_msg = KEY_ON << 8 | 1;
        key_matrix[0][10] = 0;
        if(key_msg_queue != NULL)
            xQueueSendFromISR(key_msg_queue, &key_msg, NULL);
    }
    //uartdbg_printf("key on irq:%x\n",tmp_tog & status);
}

void keyboard_init() {
    unsigned int tmp_DOUT, tmp_DOE;

    BF_CS6(
        PINCTRL_MUXSEL3,
        BANK1_PIN22, 3,
        BANK1_PIN23, 3,
        BANK1_PIN24, 3,
        BANK1_PIN25, 3,
        BANK1_PIN26, 3,
        BANK1_PIN27, 3);
    BF_CS6(
        PINCTRL_MUXSEL4,
        BANK2_PIN02, 3,
        BANK2_PIN03, 3,
        BANK2_PIN04, 3,
        BANK2_PIN05, 3,
        BANK2_PIN06, 3,
        BANK2_PIN07, 3);
    BF_CS1(
        PINCTRL_MUXSEL4,
        BANK2_PIN08, 3);

    BF_CS1(
        PINCTRL_MUXSEL4,
        BANK2_PIN14, 3);

    BF_CS1(
        PINCTRL_MUXSEL0,
        BANK0_PIN14, 3);

    BF_CS1(
        PINCTRL_MUXSEL1,
        BANK0_PIN20, 3);

    //设置引脚复用寄存器，作为默认IO口使用

    //列线设置
    tmp_DOUT = BF_RD(PINCTRL_DOUT1, DOUT);
    tmp_DOE = BF_RD(PINCTRL_DOE1, DOE);
    tmp_DOUT |= ((1 << 22) | (1 << 23) | (1 << 25) | (1 << 26) | (1 << 27));
    tmp_DOE &= ~((1 << 22) | (1 << 23) | (1 << 25) | (1 << 26) | (1 << 27));
    BF_CS1(PINCTRL_DOUT1, DOUT, tmp_DOUT);
    BF_CS1(PINCTRL_DOE1, DOE, tmp_DOE);
    //BF_CS1(PINCTRL_DOUT1,DOUT,tmp_DOUT);

    //行线设置
    tmp_DOUT = BF_RD(PINCTRL_DOUT2, DOUT);
    tmp_DOE = BF_RD(PINCTRL_DOE2, DOE);
    tmp_DOUT &= ~((1 << 14) | (1 << 8) | (1 << 7) | (1 << 6) | (1 << 5) | (1 << 4) | (1 << 3) | (1 << 2));
    tmp_DOE |= ((1 << 14) | (1 << 8) | (1 << 7) | (1 << 6) | (1 << 5) | (1 << 4) | (1 << 3) | (1 << 2));
    BF_CS1(PINCTRL_DOUT2, DOUT, tmp_DOUT);
    BF_CS1(PINCTRL_DOE2, DOE, tmp_DOE);
    BF_CS1(PINCTRL_DOUT2, DOUT, tmp_DOUT);

    tmp_DOUT = BF_RD(PINCTRL_DOUT1, DOUT);
    tmp_DOE = BF_RD(PINCTRL_DOE1, DOE);
    tmp_DOUT &= ~((1 << 24));
    tmp_DOE |= ((1 << 24));
    BF_CS1(PINCTRL_DOUT1, DOUT, tmp_DOUT);
    BF_CS1(PINCTRL_DOE1, DOE, tmp_DOE);
    BF_CS1(PINCTRL_DOUT1, DOUT, tmp_DOUT);

    tmp_DOUT = BF_RD(PINCTRL_DOUT0, DOUT);
    tmp_DOE = BF_RD(PINCTRL_DOE0, DOE);
    tmp_DOUT &= ~((1 << 20));
    tmp_DOE |= ((1 << 20));
    BF_CS1(PINCTRL_DOUT0, DOUT, tmp_DOUT);
    BF_CS1(PINCTRL_DOE0, DOE, tmp_DOE);
    BF_CS1(PINCTRL_DOUT0, DOUT, tmp_DOUT);

    //ON键
    tmp_DOUT = BF_RD(PINCTRL_DOUT0, DOUT);
    tmp_DOE = BF_RD(PINCTRL_DOE0, DOE);
    tmp_DOUT |= ((1 << 14));
    tmp_DOE &= ~((1 << 14));
    BF_CS1(PINCTRL_DOUT0, DOUT, tmp_DOUT);
    BF_CS1(PINCTRL_DOE0, DOE, tmp_DOE);
    //BF_CS1(PINCTRL_DOUT0, DOUT, tmp_DOUT);

    //deassert_all_row_line();
    //set_row_line(9);
    BF_CS1(PINCTRL_IRQLEVEL1,IRQLEVEL, 0);
    //BF_CS1(PINCTRL_IRQPOL1,IRQPOL, ((1 << 22) | (1 << 23) | (1 << 25) | (1 << 26) | (1 << 27))  );
    BF_CLR(PINCTRL_IRQPOL1,IRQPOL);
    BF_CLR(PINCTRL_IRQSTAT1,IRQSTAT);
    BF_CS1(PINCTRL_PIN2IRQ1,PIN2IRQ, ((1 << 22) | (1 << 23) | (1 << 25) | (1 << 26) | (1 << 27)) );
    BF_CS1(PINCTRL_IRQEN1,IRQEN,((1 << 22) | (1 << 23) | (1 << 25) | (1 << 26) | (1 << 27)) );

    BF_CS1(PINCTRL_IRQLEVEL0,IRQLEVEL, 0);
    BF_CLR(PINCTRL_IRQPOL0,IRQPOL);
    BF_CLR(PINCTRL_IRQSTAT0,IRQSTAT);
    BF_CS1(PINCTRL_PIN2IRQ0,PIN2IRQ, ((1 << 14)));
    BF_CS1(PINCTRL_IRQEN0,IRQEN, ((1 << 14)));


    irq_install_service(HW_IRQ_GPIO1,(unsigned int *)pinctrl1_bank1_isr);
    irq_install_service(HW_IRQ_GPIO0,(unsigned int *)pinctrl0_key_on_isr);
    irq_set_enable(HW_IRQ_GPIO1,1);
    irq_set_enable(HW_IRQ_GPIO0,1);
}

unsigned int is_key_ON_down() {
    return key_matrix[0][10];
}
/*
void key_scan() {

    for (int y = 0; y < 10; y++) {
        set_row_line(y);
        for (int x = 0; x < 5; x++) {
            key_matrix[x][y] = !read_col_line(x);
        }
    }

    key_matrix[0][10] = ((BF_RD(PINCTRL_DIN0, DIN) >> 14) & 1);
};
*/

unsigned int is_key_down(keys key) { 
    return key_matrix[key >> 3][key % 8]; 
};




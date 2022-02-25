
#include "keyboard_up.h"

#include "regspinctrl.h"

#include "../debug.h"

uint8_t key_matrix[5][11] = {0};
uint8_t key_matrix_last[5][11] = {0};

Keys_t ChangedKey = 255;



void portKeyboardGPIOInit()
{
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

    //col setting
    tmp_DOUT = BF_RD(PINCTRL_DOUT1, DOUT);
    tmp_DOE = BF_RD(PINCTRL_DOE1, DOE);
    tmp_DOUT |= ((1 << 22) | (1 << 23) | (1 << 25) | (1 << 26) | (1 << 27));
    tmp_DOE &= ~((1 << 22) | (1 << 23) | (1 << 25) | (1 << 26) | (1 << 27));
    BF_CS1(PINCTRL_DOUT1, DOUT, tmp_DOUT);
    BF_CS1(PINCTRL_DOE1, DOE, tmp_DOE);

    //row setting
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

    //key ON
    tmp_DOUT = BF_RD(PINCTRL_DOUT0, DOUT);
    tmp_DOE = BF_RD(PINCTRL_DOE0, DOE);
    tmp_DOUT |= ((1 << 14));
    tmp_DOE &= ~((1 << 14));
    BF_CS1(PINCTRL_DOUT0, DOUT, tmp_DOUT);
    BF_CS1(PINCTRL_DOE0, DOE, tmp_DOE);
    
}

static void set_row_line(int row_line) {
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


static unsigned int read_col_line(int col_line) {
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

Keys_t portGetChangedKey()
{
    Keys_t ret;
    if(ChangedKey == 255){
        return 255;
    }

    key_matrix_last[ChangedKey % 8][ChangedKey >> 3] = key_matrix[ChangedKey % 8][ChangedKey >> 3]; 

    ret = ChangedKey;
    ChangedKey = 255;
    return ret;
}

bool portIsKeyDown(Keys_t key) 
{ 
    return key_matrix[key % 8][key >> 3]; 
}

void portKeyScan() 
{

    for (int y = 0; y < 10; y++) {
        set_row_line(y);
        for (int x = 0; x < 5; x++) {
            key_matrix[x][y] = !read_col_line(x);
            if(key_matrix_last[x][y] != key_matrix[x][y])
            {   
                if(ChangedKey == 255)
                    ChangedKey = (y << 3) + x;
            }
        }
    }

    key_matrix[0][10] = ((BF_RD(PINCTRL_DIN0, DIN) >> 14) & 1);

    if(key_matrix_last[0][10] != key_matrix[0][10])
    {
        if(ChangedKey == 255)
            ChangedKey = (10 << 3) + 0;
    }

};


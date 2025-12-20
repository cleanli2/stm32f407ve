#include "stm32f4xx_hal.h"
#include "common.h"

#define delay_ms HAL_Delay
extern I2C_HandleTypeDef hi2c2;
int cam_w_reg(uint8_t addr, uint8_t data)
{
    uint8_t d[2];
    d[0]=addr;
    d[0]=data;
    return HAL_I2C_Master_Transmit(&hi2c2, 0x42, d, 2, 1000000);
}
int cam_r_regn(uint8_t addr,uint8_t n,uint8_t * buf)
{
    return 0;
}
void cam_init()
{
{
    int rcam_rty=9;
    int clks=100;


    delay_ms(20);
    lprintf("cam reset return %x\n", cam_w_reg(0x12, 0x80));
    delay_ms(20);

#if 0
    //read cam id
    while(1){
        if(0x76==cam_r_reg(0x0A)){
            break;
        }
        else{
            rcam_rty--;
            if(rcam_rty==0){
                return -1;
            }
        }
        lprintf("cam read 0x0A=%b\n", cam_r_reg(0x0A));
        delay_ms(10);
    }
    lprintf("cam read 0x0A=%b\n", cam_r_reg(0x0A));
    lprintf("cam read 0x0B=%b\n", cam_r_reg(0x0B));
    while(clks--){
        GPIO_SetBits(AL422_WG,RCK);
        delay_ms(2);
        GPIO_ResetBits(AL422_WG,RCK);
        delay_ms(2);
    }
    switch(choose){
        case 1:
            lprintf_time("set_OV7670reg\n");
            set_OV7670reg();
            break;
            //OV7670_config_window(272,12,320,240);//
        case 2:
            lprintf_time("init_rgb565_qvga_12fps\n");
            init_rgb565_qvga_12fps();
            break;
        case 3:
            lprintf_time("init_rgb565_qvga_25fps_new\n");
            init_rgb565_qvga_25fps_new();
            break;
        case 4:
            lprintf_time("init_rgb565_qvga_25fps\n");
            init_rgb565_qvga_25fps();
            break;
        case 5:
            lprintf_time("init_yuv_25fps\n");
            init_yuv_25fps();
            break;
        case 6:
            lprintf_time("init_yuv_12fps\n");
            init_yuv_12fps();
            break;
        case 7:
            lprintf_time("modified set_OV7670reg\n");
            set_OV7670reg_M();
            break;
        default:
            lprintf("cam w 0x1e return %x\n", cam_w_reg(0x1e, 0x30|cam_r_reg(0x1e)));
            lprintf_time("no init regs\n");
    }
    if(g_cam_r70p_e)lprintf("cam w 0x70 return %x\n", cam_w_reg(0x70, 0x80|cam_r_reg(0x70)));
    if(g_cam_r71p_e)lprintf("cam w 0x71 return %x\n", cam_w_reg(0x71, 0x80|cam_r_reg(0x71)));
    lprintf("cam read 0x12=%b\n", cam_r_reg(0x12));
    return 0;
#endif
}
}

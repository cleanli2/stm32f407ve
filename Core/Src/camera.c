#include "stm32f4xx_hal.h"
#include "common.h"

#define delay_ms HAL_Delay
extern I2C_HandleTypeDef hi2c2;
int cam_w_reg(uint8_t addr, uint8_t data)
{
    uint8_t d[2];
    d[0]=addr;
    d[1]=data;
    return HAL_I2C_Master_Transmit(&hi2c2, 0x42, d, 2, 1000000);
}
int cam_r_regn(uint8_t addr,uint8_t n,uint8_t * buf)
{
    return HAL_I2C_Master_Receive(&hi2c2, 0x42, buf, n, 1000000);
}
int camreaderr=0;
uint8_t cam_r_reg(uint8_t addr)
{
    uint8_t ret=0;
    camreaderr=HAL_I2C_Master_Transmit(&hi2c2, 0x42, &addr, 1, 1000000);
    if(0!=camreaderr){
        return 0;
    }
    camreaderr=cam_r_regn(addr, 1, &ret);
    if(camreaderr!=0)lprintf("cam read error\r\n");
    return ret;
}
int cam_init()
{
    int rcam_rty=9;
    uint8_t tmpd=0;


    delay_ms(20);
    lprintf("cam reset return %x\r\n", cam_w_reg(0x12, 0x80));
    delay_ms(20);

    //read cam id
    while(1){
        tmpd=cam_r_reg(0x0A);
        prt_hex(tmpd);
        prt_hex(camreaderr);
        if(0x76==tmpd){
            lprintf("cam ID OK\r\n");
            break;
        }
        else{
            rcam_rty--;
            if(rcam_rty==0){
                lprintf("cam ID fail\r\n");
                return -1;
            }
        }
        delay_ms(10);
    }
#if 0
    lprintf("cam read 0x0A=%b\n", cam_r_reg(0x0A));
    lprintf("cam read 0x0B=%b\n", cam_r_reg(0x0B));
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
    return 0;
}

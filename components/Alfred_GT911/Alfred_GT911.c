//
// Created by taxue on 2023/1/19.
//

#include "Alfred_GT911.h"

#define I2C_MASTER_FREQ_HZ          100000
#define I2C_MASTER_TIMEOUT_MS       1000

/**
 * @brief GT911 写入数据包
 * @param alfredGt911 类实例
 * @param reg 寄存器地址
 * @param data 数据
 * @param len 数据长度
 * @return 发送状态esp_err_t
 */
int GT911_write_regs(Alfred_GT911 * alfredGt911, uint16_t reg, uint8_t *data, uint8_t len)
{
    esp_err_t i2c_err;

    uint8_t regl = reg & 0xff;
    uint8_t regh = (reg>>8) & 0xff;
    uint8_t * write_package = malloc((len + 2) * sizeof(uint8_t));
    memcpy(write_package, &regh, 1);
    memcpy(write_package + 1, &regl, 1);
    memcpy(write_package + 2, data, len);

    i2c_err = i2c_master_write_to_device(alfredGt911->i2c_num,
                                         alfredGt911->gt911_addr,
                                         write_package, len + 2,
                                         I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);

    free(write_package);
    return i2c_err;
}

/**
 * @brief GT911 发送数据包
 * @param alfredGt911 类实例
 * @param reg 寄存器地址
 * @param data 数据
 * @param len 数据长度
 * @return 发送读取状态esp_err_t
 */
int GT911_read_regs(Alfred_GT911 * alfredGt911, uint16_t reg, uint8_t *data, uint8_t len)
{
    uint8_t regl = reg&0xff;
    uint8_t regh = (reg>>8)&0xff;
    uint8_t rbuf[2] = {regh, regl};

    return i2c_master_write_read_device(alfredGt911->i2c_num, alfredGt911->gt911_addr,
                                        rbuf, 2, data, len,
                                        I2C_MASTER_TIMEOUT_MS / portTICK_PERIOD_MS);
}

/**
 * @brief GT911 初始化函数
 * @param alfredGt911 类实例
 * @param SDA SDA引脚
 * @param SCL SCL引脚
 * @param INT 中断引脚，没有赋值-1
 * @param RES 重置引脚，没有赋值-1
 * @param i2c_num I2C端口号
 * @param gt911_addr GT911地址
 * @param width 屏幕宽度
 * @param height 屏幕高度
 */
void GT911_init(Alfred_GT911 * alfredGt911, int SDA, int SCL, int INT, int RES,
                i2c_port_t i2c_num, uint8_t gt911_addr, uint16_t width, uint16_t height)
{
    alfredGt911->gt911_i2c_config.mode = I2C_MODE_MASTER;
    alfredGt911->gt911_i2c_config.sda_io_num = SDA;
    alfredGt911->gt911_i2c_config.sda_pullup_en = GPIO_PULLUP_ENABLE;
    alfredGt911->gt911_i2c_config.scl_io_num = SCL;
    alfredGt911->gt911_i2c_config.scl_pullup_en = GPIO_PULLUP_ENABLE;
    alfredGt911->gt911_i2c_config.master.clk_speed = I2C_MASTER_FREQ_HZ;

    alfredGt911->i2c_num = i2c_num;
    alfredGt911->gt911_addr = gt911_addr;
    alfredGt911->height = height;
    alfredGt911->width = width;

    i2c_param_config(i2c_num, &alfredGt911->gt911_i2c_config);

    i2c_driver_install(i2c_num, alfredGt911->gt911_i2c_config.mode, 0,
                       0, 0);

    uint8_t buf[5] = {0};
    GT911_read_regs(alfredGt911, GT911_PRODUCT_ID, buf, 4);
    printf("GT911 PRODUCT ID: %s\n", buf);
}

/**
 * @brief 设置屏幕方向
 * @param alfredGt911 类实例
 * @param rot 方向
 */
void GT911_setRotation(Alfred_GT911 * alfredGt911, uint8_t rot)
{
    alfredGt911->rotation = rot;
}

/**
 * @brief 检测是否被触摸并且获取相关值
 * @param alfredGt911
 * @return 触摸为true，反之
 */
bool GT911_touched(Alfred_GT911 * alfredGt911)
{
    uint8_t touched_state, touch_num, buffer_status;
    GT911_read_regs(alfredGt911, GT911_POINT_INFO, &touched_state, 1);
    touch_num = touched_state & 0xf; //触点数量
    buffer_status = (touched_state >> 7) & 1; // 帧状态

    if(buffer_status == 1 && (touch_num <= TOUCH_POINT_TOTAL) && (touch_num > 0)){
        uint16_t POINTERS_REGS[TOUCH_POINT_TOTAL] = {GT911_POINT_1, GT911_POINT_2, GT911_POINT_3, GT911_POINT_4, GT911_POINT_5};
        // 获取每个触控点的坐标值并保存
        for (int i = 0; i < TOUCH_POINT_TOTAL; ++i) {
            uint8_t point_info_per_size = 7;
            uint8_t * point_info_p = malloc(point_info_per_size * sizeof(uint8_t ));
            GT911_read_regs(alfredGt911, POINTERS_REGS[i], point_info_p, point_info_per_size);

            alfredGt911->points_info[i].id = point_info_p[0];
            alfredGt911->points_info[i].x = point_info_p[1] + (point_info_p[2] << 8);
            alfredGt911->points_info[i].y = point_info_p[3] + (point_info_p[4] << 8);
            alfredGt911->points_info[i].size = point_info_p[5] + (point_info_p[6] << 8);

            free(point_info_p);

            //旋转方向
            uint16_t temp;
            switch (alfredGt911->rotation){
                case ROTATION_NORMAL:
                    alfredGt911->points_info[i].x = alfredGt911->width - alfredGt911->points_info[i].x;
                    alfredGt911->points_info[i].y = alfredGt911->height - alfredGt911->points_info[i].y;
                    break;
                case ROTATION_LEFT:
                    temp = alfredGt911->points_info[i].x;
                    alfredGt911->points_info[i].x = alfredGt911->width - alfredGt911->points_info[i].y;
                    alfredGt911->points_info[i].y = temp;
                    break;
                case ROTATION_INVERTED:
                    alfredGt911->points_info[i].x = alfredGt911->points_info[i].x;
                    alfredGt911->points_info[i].y = alfredGt911->points_info[i].y;
                    break;
                case ROTATION_RIGHT:
                    temp = alfredGt911->points_info[i].x;
                    alfredGt911->points_info[i].x = alfredGt911->points_info[i].y;
                    alfredGt911->points_info[i].y = alfredGt911->height - temp;
                    break;
                default:
                    break;
            }
        }
        uint8_t temp = 0;
        GT911_write_regs(alfredGt911, GT911_POINT_INFO, &temp, 1);
        return true;
    }
    uint8_t temp = 0;
    //必须给GT911_POINT_INFO缓冲区置0,不然读取的数据一直为128！！！！
    GT911_write_regs(alfredGt911, GT911_POINT_INFO, &temp, 1);
    return false;
}

/**
 * @brief 获取每个触控点的位置
 * @param alfredGt911 类实例
 * @param x 被修改的x值
 * @param y 被修改的y值
 * @param index 触控点下标 [0-4]
 */
void GT911_read_pos(Alfred_GT911 * alfredGt911, int *x, int *y, int index)
{
    *x = alfredGt911->points_info[index].x;
    *y = alfredGt911->points_info[index].y;
}
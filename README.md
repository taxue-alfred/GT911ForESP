# GT911ForESP
GT911在ESP系列上的驱动，基于IDF5.0，ESP32S3编写

本库使用面向对象思想编写，可创建多设备多实例

Github，Gitee同步更新，Gitee仅作为下载仓库，提交Issue和Pull request请到Github

[Github: https://github.com/taxue-alfred/GT911ForESP](https://github.com/taxue-alfred/GT911ForESP)

[Gitee: https://gitee.com/MediaState/MediaStateT](https://gitee.com/MediaState/MediaStateT)

## 1. IDF 4.4 是否可用?

理论可用，未经实践

## 2. 注意点

> 以下内容基于编程开发手册REV11.0

### 1. 0x8040寄存器貌似不再支持软重启

> 如果按照旧手册使用软重启，读取0x814E会一直返回128(0x80)，即使对寄存器写入0。也是坑，如果你买的设备使用的芯片出厂较早，那么网上一些驱动可用。但是我是新芯片，没错又浪费时间了。。。

![image-20230120202422455](https://taxue-alfred-1253400076.cos.ap-beijing.myqcloud.com/image-20230120202422455.png)

### 2. 0x814E状态寄存器必须在读取完或开机之后写入0再读取

> 这个挺坑的，我就被坑了。。一开始找到的是旧的编程文档，并没有说明这一项。。时间被白白浪费了。。。

![image-20230120202625308](https://taxue-alfred-1253400076.cos.ap-beijing.myqcloud.com/image-20230120202625308.png)

## 3. 目录结构

```
├─components
│  └─Alfred_GT911
│      │  Alfred_GT911.c 驱动文件
│      │  CMakeLists.txt
│      │
│      └─includes
│              Alfred_GT911.h
│
└─main
        CMakeLists.txt
        GT911ForESP.c 示例文件
```

## 4. 如何移植？

### 1. Alfred_GT911结构体修改

```c
/**类结构体**/
//根据不同平台修改此项
typedef struct {
    i2c_config_t gt911_i2c_config; 
    i2c_port_t i2c_num;
    uint8_t gt911_addr;
    uint16_t height;
    uint16_t width;
    uint8_t rotation;
    TP_point_info points_info[TOUCH_POINT_TOTAL]; //用于存储五个触控点的坐标
}Alfred_GT911;
```

### 2. GT911_write_regs() & GT911_read_regs()修改

这两个函数定义了数据发送结构，构成其读写协议。

> 编写协议函数时注意传入类实例指针，不传入也可以，不过上面的结构体就没有意义了

不同平台按照以下形式构建：

![image-20230120203507285](https://taxue-alfred-1253400076.cos.ap-beijing.myqcloud.com/image-20230120203507285.png)

### 3. 日志打印函数修改

GT911_init()函数中：

```c
    uint8_t buf[5] = {0};
    GT911_read_regs(alfredGt911, GT911_PRODUCT_ID, buf, 4);
    printf("GT911 PRODUCT ID: %s\n", buf);
```

替换成平台对应的打印函数，如果不想要，删除即可。

## 5. 参考驱动

[https://github.com/TAMCTec/gt911-arduino](https://github.com/TAMCTec/gt911-arduino)

[https://gitee.com/andida/esp32s3-gt911](https://gitee.com/andida/esp32s3-gt911) 

> 此库在我的芯片不可用，修改后可用，已在库中提交Pull Request


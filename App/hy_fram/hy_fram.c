#include "hy_fram.h"
#include "hy_flash.h"

slot_t ram;   	// 创建数据缓存结构体
static int16_t group_buf[300*20];

uint8_t __fram_read_write(uint8_t byte)
{
    uint8_t receivedByte = 0;

    // 发送数据并接收响应
    HAL_SPI_TransmitReceive(&hspi1, &byte, &receivedByte, 1, HAL_MAX_DELAY);

    return receivedByte;
}

void fram_spi_write(uint32_t addr, uint8_t *data, uint16_t len)
{
	FRAM_CS_L();
	__fram_read_write(FRAM_WREN);		//发送写使能
	FRAM_CS_H();
	
	FRAM_CS_L();
    uint8_t cmd[4] = {FRAM_WRITE, (addr>>16)&0x01, (addr>>8)&0xFF, addr&0xFF};	//写存储器+17位地址
    HAL_SPI_Transmit(&hspi1, cmd, 4, HAL_MAX_DELAY);
    HAL_SPI_Transmit(&hspi1, data, len, HAL_MAX_DELAY);
    FRAM_CS_H();
}

void fram_spi_read(uint32_t addr, uint8_t *data, uint16_t len)
{
	FRAM_CS_L();

	uint8_t cmd[4] = {FRAM_READ, (addr>>16)&0x01, (addr>>8)&0xFF, addr&0xFF};	//读存储器+17位地址
    HAL_SPI_Transmit(&hspi1, cmd, 4, HAL_MAX_DELAY);
	HAL_SPI_Receive(&hspi1, data, len, HAL_MAX_DELAY);
	
    FRAM_CS_H();
}


void fram_test(void)
{
//	uint8_t read_id[10];
//	
//	FRAM_CS_L();
//	__fram_read_write(0x9F);

//	for(uint8_t i=0; i<10; i++)
//	{
//		read_id[i] = __fram_read_write(0xFF);
//	}
//	
//	FRAM_CS_H();
}

/* 向当前槽塞一个新样本 */
void push_sample(int16_t v)
{
	ram.buf[ram.wr_idx] = v;
	ram.wr_idx = (ram.wr_idx + 1) % 300;
	if(ram.cnt < 300) ram.cnt++;
}

/* 把当前读取到的数据保存到某一槽 */
void slot_flush_reorder(uint8_t slot_idx)
{
    uint16_t temp[300] = {0};
    uint16_t cnt = ram.cnt;          // 已存有效个数

    if(cnt < 300)
	{
        /* 线性段：0 → cnt-1 */
        memcpy(temp, ram.buf, cnt * sizeof(uint16_t));
        /* 剩余补 0x00 */
        memset(&temp[cnt], 0x00, (300 - cnt) * sizeof(uint16_t));
    }
	else
	{
        /* 环形：从 wr_idx 开始循环 300 个 */
        for(uint16_t i = 0; i < 300; i++)
		{
            uint16_t ri = (ram.wr_idx + i) % 300;
            temp[i] = ram.buf[ri];
        }
    }

    uint32_t addr = slot_idx * BLK_SZ;
    fram_spi_write(addr, (uint8_t*)temp, BLK_SZ);
}

/* 用户切换新组 */
void user_switch_group(uint8_t new_g)
{
    if(new_g >= 99) return;

	/* 1. 若是组内数据有过增删则写入flash*/
	if(ram.modified)
	{
		ram.modified = 0;
		fram_spi_read(0x0000, (uint8_t *)group_buf, GROUP_SZ*BLK_SZ);		//读取存储在fram的一整组数据
		w25q64_write(GROUP_DATA_ADDR(ram.cur_group), (uint8_t *)group_buf, GROUP_SZ*BLK_SZ);
	}
		
	/* 2. 读取当前组内数据放入fram，方便增删操作，组内有多少槽数据就读多少 */
	memset(group_buf, 0x00, GROUP_SZ*BLK_SZ);		//group_buf清零防止没被覆盖的部分有废弃数据
	w25q64_read(GROUP_DATA_ADDR(new_g), (uint8_t *)group_buf, (ram.cur_slot)*BLK_SZ);
	fram_spi_write(0x0000, (uint8_t*)group_buf, GROUP_SZ*BLK_SZ);

    /* 3. 切到新组号 */
    ram.cur_group = new_g;
}

/* 用户保存数据到当前槽并切换到下一槽 */
void user_switch_slot(void)
{
	if(ram.cur_slot > 19) return;
	
    /* 1. 把 RAM 里最新 300 个（重排后）写进当前 FRAM 槽 */
    slot_flush_reorder(ram.cur_slot);

    /* 2. 切到下一个空槽,cur_slot=20表示当前组存满(0~19) */
    ram.cur_slot++;

    /* 3. RAM 缓冲清零，重新积累 */
    ram.wr_idx = 0;
    ram.cnt    = 0;
	
	ram.modified = 1;	// 数据修改标志位置1
}

/* 用户删除单个数据 */
void user_delete_slot(uint8_t slot_idx)
{
    if(slot_idx >= ram.cur_slot) return;	//不能删除正在填充及后面的槽

    /* 1. 把 [slot_idx+1 .. 19] 整体向前搬 1 个 phys 位置 */
    uint8_t buf[BLK_SZ];                       // 600 B 临时缓冲
    for(uint8_t i = slot_idx; i < ram.cur_slot-1; i++){
        uint32_t src = (i+1) * BLK_SZ;
        uint32_t dst = i * BLK_SZ;
        fram_spi_read(src, buf, BLK_SZ);
        fram_spi_write(dst, buf, BLK_SZ);
    }

    /* 2. 最后一槽清空（现在空出来） */
    uint32_t last = (ram.cur_slot-1) * BLK_SZ;
    memset(buf, 0x00, BLK_SZ);
    fram_spi_write(last, buf, BLK_SZ);

    /* 3. cur_slot减 1（保持 phys 连续） */
    ram.cur_slot--;
	
	ram.modified = 1;	// 数据修改标志位置1
}

/* 用户删除整组数据 */
void user_delete_group(void)
{
	ram.cur_slot   = 0;
	memset(group_buf, 0x00, GROUP_SZ*BLK_SZ);
	fram_spi_write(0x0000, (uint8_t *)group_buf,GROUP_SZ*BLK_SZ);	//fram清零以便接收下一组数据
	ram.modified = 1;	// 数据修改标志位置1
}

void fram_init(void)
{
	ram.cur_group  = 0;
	ram.cur_slot   = 0;
    ram.wr_idx     = 0;
    ram.cnt        = 0;
	ram.modified   = 0;
	memset(ram.buf, 0x00, BLK_SZ);
	memset(group_buf, 0x00, GROUP_SZ*BLK_SZ);
	fram_spi_write(0x0000, (uint8_t *)group_buf,GROUP_SZ*BLK_SZ);
}

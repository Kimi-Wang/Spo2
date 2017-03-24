#ifndef _COMMANDHANDLE_H
#define _COMMANDHANDLE_H

#include <stdint.h>
#include "app_mailbox.h"
#include "ble_nus.h"

// BLE_Uart发送消息队列

#define BLE_NOTIFY_QUEUE_SIZE 150
typedef struct {
    uint8_t len;
    uint8_t msg[BLE_NUS_MAX_DATA_LEN];
} ble_notify_msg_t;



#define MAX_MSG_PDU_SIZE 20

typedef struct {
    uint8_t len;

    uint8_t msg[MAX_MSG_PDU_SIZE];

} th_ble_msg_evt_hdr_t;

typedef void (*notify_free_handler_t)(void );

extern uint32_t sample_info_buffer[10][4];

extern uint8_t erase_all_data_flag;

extern ret_code_t notify_msg_send(uint8_t *data, int len);

extern uint32_t count_total_sample_num(uint32_t * start_storage_addr);

void msg_event_handler(uint8_t *p_data, uint16_t length);

#endif

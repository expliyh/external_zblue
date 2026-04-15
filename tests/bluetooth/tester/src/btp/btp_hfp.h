/* btp_hfp.h - Bluetooth HFP tester headers */

/*
 * Copyright (c) 2026 XiaoMi
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/sys/util.h>
#include <zephyr/bluetooth/addr.h>

/* HFP Service */
/* commands */
#define BTP_HFP_READ_SUPPORTED_COMMANDS		0x01
struct btp_hfp_read_supported_commands_rp {
	uint8_t data[0];
} __packed;

#define BTP_HFP_CONNECT				0x02
struct btp_hfp_connect_cmd {
	bt_addr_le_t address;
} __packed;

#define BTP_HFP_DISCONNECT			0x03
struct btp_hfp_disconnect_cmd {
	bt_addr_le_t address;
} __packed;

#define BTP_HFP_ANSWER_CALL			0x04
struct btp_hfp_answer_call_cmd {
	bt_addr_le_t address;
} __packed;

#define BTP_HFP_REJECT_CALL			0x05
struct btp_hfp_reject_call_cmd {
	bt_addr_le_t address;
} __packed;

#define BTP_HFP_DIAL				0x06
struct btp_hfp_dial_cmd {
	bt_addr_le_t address;
	uint8_t number_len;
	uint8_t number[];
} __packed;

#define BTP_HFP_SET_VOLUME			0x07
struct btp_hfp_set_volume_cmd {
	bt_addr_le_t address;
	uint8_t type;
	uint8_t volume;
} __packed;

#define BTP_HFP_SEND_DTMF			0x08
struct btp_hfp_send_dtmf_cmd {
	bt_addr_le_t address;
	uint8_t code;
} __packed;

#define BTP_HFP_CONNECT_ACL			0x09
struct btp_hfp_connect_acl_cmd {
	bt_addr_le_t address;
} __packed;

#define BTP_HFP_CONNECT_AUDIO			0x0a
struct btp_hfp_connect_audio_cmd {
	bt_addr_le_t address;
} __packed;

#define BTP_HFP_DISCONNECT_AUDIO		0x0b
struct btp_hfp_disconnect_audio_cmd {
	bt_addr_le_t address;
} __packed;

#define BTP_HFP_TERMINATE_CALL			0x0c
struct btp_hfp_terminate_call_cmd {
	bt_addr_le_t address;
} __packed;

#define BTP_HFP_QUERY_CURRENT_CALLS		0x0d
struct btp_hfp_query_current_calls_cmd {
	bt_addr_le_t address;
} __packed;

#define BTP_HFP_GET_SUBSCRIBER_NUMBER		0x0e
struct btp_hfp_get_subscriber_number_cmd {
	bt_addr_le_t address;
} __packed;

#define BTP_HFP_START_VOICE_RECOGNITION		0x0f
struct btp_hfp_start_voice_recognition_cmd {
	bt_addr_le_t address;
} __packed;

#define BTP_HFP_STOP_VOICE_RECOGNITION		0x10
struct btp_hfp_stop_voice_recognition_cmd {
	bt_addr_le_t address;
} __packed;

#define BTP_HFP_CALL_CONTROL			0x11
struct btp_hfp_call_control_cmd {
	bt_addr_le_t address;
	uint8_t chld;
	uint8_t index;
} __packed;

#define BTP_HFP_UPDATE_BATTERY_LEVEL		0x12
struct btp_hfp_update_battery_level_cmd {
	bt_addr_le_t address;
	uint8_t level;
} __packed;

#define BTP_HFP_QUERY_CALLS			0x0d
struct btp_hfp_query_calls_cmd {
	bt_addr_le_t address;
} __packed;

#define BTP_HFP_SEND_AT_CMD			0x0d
struct btp_hfp_send_at_cmd_cmd {
	bt_addr_le_t address;
	uint8_t cmd_len;
	uint8_t cmd[];
} __packed;

/* events */
#define BTP_HFP_EV_CONNECTED			0x80
struct btp_hfp_connected_ev {
	bt_addr_le_t address;
} __packed;

#define BTP_HFP_EV_DISCONNECTED			0x81
struct btp_hfp_disconnected_ev {
	bt_addr_le_t address;
} __packed;

#define BTP_HFP_EV_CALL_STATE			0x82
struct btp_hfp_call_state_ev {
	bt_addr_le_t address;
	uint8_t state;
} __packed;

#define BTP_HFP_EV_AUDIO_STATE			0x83
struct btp_hfp_audio_state_ev {
	bt_addr_le_t address;
	uint8_t state;
} __packed;

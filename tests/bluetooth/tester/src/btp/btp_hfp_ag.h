/* btp_hfp_ag.h - Bluetooth HFP AG tester headers */

/*
 * Copyright (c) 2026 XiaoMi
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/sys/util.h>
#include <zephyr/bluetooth/addr.h>

/* HFP AG Service - BTP_SERVICE_ID_HFP_AG = 0x27 */

/* commands */
#define BTP_HFP_AG_READ_SUPPORTED_COMMANDS	0x01
struct btp_hfp_ag_read_supported_commands_rp {
	uint8_t data[0];
} __packed;

#define BTP_HFP_AG_CONNECT			0x02
struct btp_hfp_ag_connect_cmd {
	bt_addr_le_t address;
} __packed;

#define BTP_HFP_AG_DISCONNECT			0x03
struct btp_hfp_ag_disconnect_cmd {
	bt_addr_le_t address;
} __packed;

#define BTP_HFP_AG_CONNECT_AUDIO		0x04
struct btp_hfp_ag_connect_audio_cmd {
	bt_addr_le_t address;
} __packed;

#define BTP_HFP_AG_DISCONNECT_AUDIO		0x05
struct btp_hfp_ag_disconnect_audio_cmd {
	bt_addr_le_t address;
} __packed;

#define BTP_HFP_AG_PHONE_STATE_CHANGE		0x06
struct btp_hfp_ag_phone_state_change_cmd {
	bt_addr_le_t address;
	uint8_t num_active;
	uint8_t num_held;
	uint8_t call_state;
	uint8_t addr_type;
	uint8_t number_len;
	uint8_t number[];
} __packed;

#define BTP_HFP_AG_VOLUME_CONTROL		0x07
struct btp_hfp_ag_volume_control_cmd {
	bt_addr_le_t address;
	uint8_t type;
	uint8_t volume;
} __packed;

#define BTP_HFP_AG_START_VR			0x08
struct btp_hfp_ag_start_vr_cmd {
	bt_addr_le_t address;
} __packed;

#define BTP_HFP_AG_STOP_VR			0x09
struct btp_hfp_ag_stop_vr_cmd {
	bt_addr_le_t address;
} __packed;

#define BTP_HFP_AG_DEVICE_STATUS		0x0a
struct btp_hfp_ag_device_status_cmd {
	bt_addr_le_t address;
	uint8_t network;
	uint8_t roam;
	uint8_t signal;
	uint8_t battery;
} __packed;

#define BTP_HFP_AG_CLCC_RESPONSE		0x0b
struct btp_hfp_ag_clcc_response_cmd {
	bt_addr_le_t address;
	uint32_t index;
	uint8_t dir;
	uint8_t call_state;
	uint8_t mode;
	uint8_t mpty;
	uint8_t addr_type;
	uint8_t number_len;
	uint8_t number[];
} __packed;

#define BTP_HFP_AG_CIND_RESPONSE		0x0c
struct btp_hfp_ag_cind_response_cmd {
	bt_addr_le_t address;
	uint8_t network;
	uint8_t call;
	uint8_t callsetup;
	uint8_t callheld;
	uint8_t signal;
	uint8_t roam;
	uint8_t battery;
} __packed;

#define BTP_HFP_AG_DIAL_RESPONSE		0x0d
struct btp_hfp_ag_dial_response_cmd {
	uint8_t result;
} __packed;

#define BTP_HFP_AG_SEND_AT_CMD			0x0e
struct btp_hfp_ag_send_at_cmd_cmd {
	bt_addr_le_t address;
	uint8_t cmd_len;
	uint8_t cmd[];
} __packed;

#define BTP_HFP_AG_SEND_VENDOR_AT_CMD		0x0f
struct btp_hfp_ag_send_vendor_at_cmd_cmd {
	bt_addr_le_t address;
	uint8_t cmd_len;
	uint8_t val_len;
	uint8_t data[];  /* cmd followed by val */
} __packed;

#define BTP_HFP_AG_START_VIRTUAL_CALL		0x10
struct btp_hfp_ag_start_virtual_call_cmd {
	bt_addr_le_t address;
} __packed;

#define BTP_HFP_AG_STOP_VIRTUAL_CALL		0x11
struct btp_hfp_ag_stop_virtual_call_cmd {
	bt_addr_le_t address;
} __packed;

#define BTP_HFP_AG_CONNECT_ACL			0x12
struct btp_hfp_ag_connect_acl_cmd {
	bt_addr_le_t address;
} __packed;

/* events */
#define BTP_HFP_AG_EV_CONNECTED		0x80
struct btp_hfp_ag_connected_ev {
	bt_addr_le_t address;
} __packed;

#define BTP_HFP_AG_EV_DISCONNECTED		0x81
struct btp_hfp_ag_disconnected_ev {
	bt_addr_le_t address;
} __packed;

#define BTP_HFP_AG_EV_AUDIO_STATE		0x82
struct btp_hfp_ag_audio_state_ev {
	bt_addr_le_t address;
	uint8_t state;
} __packed;

#define BTP_HFP_AG_EV_ANSWER_CALL		0x83
struct btp_hfp_ag_answer_call_ev {
	bt_addr_le_t address;
} __packed;

#define BTP_HFP_AG_EV_REJECT_CALL		0x84
struct btp_hfp_ag_reject_call_ev {
	bt_addr_le_t address;
} __packed;

#define BTP_HFP_AG_EV_HANGUP_CALL		0x85
struct btp_hfp_ag_hangup_call_ev {
	bt_addr_le_t address;
} __packed;

#define BTP_HFP_AG_EV_DIAL			0x86
struct btp_hfp_ag_dial_ev {
	bt_addr_le_t address;
	uint8_t number_len;
	uint8_t number[];
} __packed;

#define BTP_HFP_AG_EV_VR_STATE			0x87
struct btp_hfp_ag_vr_state_ev {
	bt_addr_le_t address;
	uint8_t started;
} __packed;

#define BTP_HFP_AG_EV_VOLUME_CONTROL		0x88
struct btp_hfp_ag_volume_control_ev {
	bt_addr_le_t address;
	uint8_t type;
	uint8_t volume;
} __packed;

#define BTP_HFP_AG_EV_CLCC_REQUEST		0x89
struct btp_hfp_ag_clcc_request_ev {
	bt_addr_le_t address;
} __packed;

#define BTP_HFP_AG_EV_CIND_REQUEST		0x8a
struct btp_hfp_ag_cind_request_ev {
	bt_addr_le_t address;
} __packed;

#define BTP_HFP_AG_EV_AT_CMD			0x8b
struct btp_hfp_ag_at_cmd_ev {
	bt_addr_le_t address;
	uint8_t cmd_len;
	uint8_t cmd[];
} __packed;

#define BTP_HFP_AG_EV_BATTERY_UPDATE		0x8d
struct btp_hfp_ag_battery_update_ev {
	bt_addr_le_t address;
	uint8_t level;
} __packed;

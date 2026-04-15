/* btp_hfp.c - Bluetooth HFP Tester */

/*
 * Copyright (c) 2026 XiaoMi
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/sys/atomic.h>
#include <zephyr/types.h>
#include <string.h>

#include <zephyr/toolchain.h>
#include <zephyr/bluetooth/bluetooth.h>
#include <zephyr/bluetooth/conn.h>

#include <zephyr/sys/byteorder.h>

#include <zephyr/logging/log.h>
#define LOG_MODULE_NAME bttester_hfp
LOG_MODULE_REGISTER(LOG_MODULE_NAME, CONFIG_BTTESTER_LOG_LEVEL);

#include "btp/btp.h"
#include "z_api_port.h"

/* Forward declarations for z_api HFP functions */
extern int z_bt_hfp_connect(const uint8_t *addr);
extern int z_bt_hfp_disconnect(const uint8_t *addr);
extern int z_bt_hfp_answer(const uint8_t *addr);
extern int z_bt_hfp_reject(const uint8_t *addr);
extern int z_bt_hfp_dial(const uint8_t *addr, const char *number);
extern int z_bt_hfp_set_volume(const uint8_t *addr,
			       uint8_t type, uint8_t volume);
extern int z_bt_hfp_send_dtmf(const uint8_t *addr, uint8_t code);
extern int z_bt_hfp_init(void);
extern int z_bt_hfp_connect_acl(const uint8_t *addr);
extern int z_bt_hfp_connect_audio(const uint8_t *addr);
extern int z_bt_hfp_disconnect_audio(const uint8_t *addr);
extern int z_bt_hfp_terminate_call(const uint8_t *addr);
extern int z_bt_hfp_get_subscriber_number(const uint8_t *addr);
extern int z_bt_hfp_start_voice_recognition(const uint8_t *addr);
extern int z_bt_hfp_stop_voice_recognition(const uint8_t *addr);
extern int z_bt_hfp_call_control(const uint8_t *addr, uint8_t chld,
				 uint8_t index);
extern int z_bt_hfp_update_battery_level(const uint8_t *addr, uint8_t level);

/* Direct zblue API for sending AT+CLCC (bypasses framework cache) */
extern int z_bt_hfp_hf_send_clcc(struct bt_conn *conn);
/* Direct zblue API for sending AT+VGM/VGS (bypasses framework volume conversion) */
extern int z_bt_hfp_hf_send_volume(struct bt_conn *conn, uint8_t type,
				    uint8_t volume);

static uint8_t supported_commands(const void *cmd, uint16_t cmd_len,
				  void *rsp, uint16_t *rsp_len)
{
	struct btp_hfp_read_supported_commands_rp *rp = rsp;

	tester_set_bit(rp->data, BTP_HFP_READ_SUPPORTED_COMMANDS);
	tester_set_bit(rp->data, BTP_HFP_CONNECT);
	tester_set_bit(rp->data, BTP_HFP_DISCONNECT);
	tester_set_bit(rp->data, BTP_HFP_ANSWER_CALL);
	tester_set_bit(rp->data, BTP_HFP_REJECT_CALL);
	tester_set_bit(rp->data, BTP_HFP_DIAL);
	tester_set_bit(rp->data, BTP_HFP_SET_VOLUME);
	tester_set_bit(rp->data, BTP_HFP_SEND_DTMF);
	tester_set_bit(rp->data, BTP_HFP_CONNECT_ACL);
	tester_set_bit(rp->data, BTP_HFP_CONNECT_AUDIO);
	tester_set_bit(rp->data, BTP_HFP_DISCONNECT_AUDIO);
	tester_set_bit(rp->data, BTP_HFP_TERMINATE_CALL);
	tester_set_bit(rp->data, BTP_HFP_QUERY_CURRENT_CALLS);
	tester_set_bit(rp->data, BTP_HFP_GET_SUBSCRIBER_NUMBER);
	tester_set_bit(rp->data, BTP_HFP_START_VOICE_RECOGNITION);
	tester_set_bit(rp->data, BTP_HFP_STOP_VOICE_RECOGNITION);
	tester_set_bit(rp->data, BTP_HFP_CALL_CONTROL);
	tester_set_bit(rp->data, BTP_HFP_UPDATE_BATTERY_LEVEL);

	*rsp_len = sizeof(*rp) + 2;

	return BTP_STATUS_SUCCESS;
}

static uint8_t hfp_connect(const void *cmd, uint16_t cmd_len,
			   void *rsp, uint16_t *rsp_len)
{
	const struct btp_hfp_connect_cmd *cp = cmd;
	int err;

	LOG_INF("HFP connect");

	err = z_bt_hfp_connect((const uint8_t *)&cp->address);
	if (err) {
		LOG_ERR("HFP connect failed: %d", err);
		return BTP_STATUS_FAILED;
	}

	return BTP_STATUS_SUCCESS;
}

static uint8_t hfp_disconnect(const void *cmd, uint16_t cmd_len,
			      void *rsp, uint16_t *rsp_len)
{
	const struct btp_hfp_disconnect_cmd *cp = cmd;
	int err;

	LOG_INF("HFP disconnect");

	err = z_bt_hfp_disconnect((const uint8_t *)&cp->address);
	if (err) {
		LOG_ERR("HFP disconnect failed: %d", err);
		return BTP_STATUS_FAILED;
	}

	return BTP_STATUS_SUCCESS;
}

static uint8_t hfp_answer_call(const void *cmd, uint16_t cmd_len,
			       void *rsp, uint16_t *rsp_len)
{
	const struct btp_hfp_answer_call_cmd *cp = cmd;
	int err;

	LOG_INF("HFP answer call");

	err = z_bt_hfp_answer((const uint8_t *)&cp->address);
	if (err) {
		LOG_ERR("HFP answer call failed: %d", err);
		return BTP_STATUS_FAILED;
	}

	return BTP_STATUS_SUCCESS;
}

static uint8_t hfp_reject_call(const void *cmd, uint16_t cmd_len,
			       void *rsp, uint16_t *rsp_len)
{
	const struct btp_hfp_reject_call_cmd *cp = cmd;
	int err;

	LOG_INF("HFP reject call");

	err = z_bt_hfp_reject((const uint8_t *)&cp->address);
	if (err) {
		LOG_ERR("HFP reject call failed: %d", err);
		return BTP_STATUS_FAILED;
	}

	return BTP_STATUS_SUCCESS;
}

static uint8_t hfp_dial(const void *cmd, uint16_t cmd_len,
			void *rsp, uint16_t *rsp_len)
{
	const struct btp_hfp_dial_cmd *cp = cmd;
	char number[33];
	int err;
	uint8_t len;

	LOG_INF("HFP dial");

	len = cp->number_len;
	if (len > 32) {
		len = 32;
	}
	memcpy(number, cp->number, len);
	number[len] = '\0';

	err = z_bt_hfp_dial((const uint8_t *)&cp->address, number);
	if (err) {
		LOG_ERR("HFP dial failed: %d", err);
		return BTP_STATUS_FAILED;
	}

	return BTP_STATUS_SUCCESS;
}

static uint8_t hfp_set_volume(const void *cmd, uint16_t cmd_len,
			      void *rsp, uint16_t *rsp_len)
{
	const struct btp_hfp_set_volume_cmd *cp = cmd;
	struct bt_conn *conn;
	int err;

	LOG_INF("HFP set volume: type=%d vol=%d", cp->type, cp->volume);

	/* Bypass framework volume conversion, send AT+VGM/VGS directly */
	conn = bt_conn_lookup_addr_br(&cp->address.a);
	if (!conn) {
		LOG_ERR("No BR/EDR connection found");
		return BTP_STATUS_FAILED;
	}

	err = z_bt_hfp_hf_send_volume(conn, cp->type, cp->volume);
	bt_conn_unref(conn);

	if (err) {
		LOG_ERR("HFP set volume failed: %d", err);
		return BTP_STATUS_FAILED;
	}

	return BTP_STATUS_SUCCESS;
}

static uint8_t hfp_send_dtmf(const void *cmd, uint16_t cmd_len,
			      void *rsp, uint16_t *rsp_len)
{
	const struct btp_hfp_send_dtmf_cmd *cp = cmd;
	int err;

	LOG_INF("HFP send DTMF: code=%c", cp->code);

	err = z_bt_hfp_send_dtmf((const uint8_t *)&cp->address, cp->code);
	if (err) {
		LOG_ERR("HFP send DTMF failed: %d", err);
		return BTP_STATUS_FAILED;
	}

	return BTP_STATUS_SUCCESS;
}

static uint8_t hfp_connect_acl(const void *cmd, uint16_t cmd_len,
			       void *rsp, uint16_t *rsp_len)
{
	const struct btp_hfp_connect_acl_cmd *cp = cmd;
	int err;

	LOG_INF("HFP connect ACL only");

	err = z_bt_hfp_connect_acl((const uint8_t *)&cp->address);
	if (err) {
		LOG_ERR("HFP connect ACL failed: %d", err);
		return BTP_STATUS_FAILED;
	}

	return BTP_STATUS_SUCCESS;
}

static uint8_t hfp_connect_audio(const void *cmd, uint16_t cmd_len,
				 void *rsp, uint16_t *rsp_len)
{
	const struct btp_hfp_connect_audio_cmd *cp = cmd;
	int err;

	LOG_INF("HFP connect audio");

	err = z_bt_hfp_connect_audio((const uint8_t *)&cp->address);
	if (err) {
		LOG_ERR("HFP connect audio failed: %d", err);
		return BTP_STATUS_FAILED;
	}

	return BTP_STATUS_SUCCESS;
}

static uint8_t hfp_disconnect_audio(const void *cmd, uint16_t cmd_len,
				    void *rsp, uint16_t *rsp_len)
{
	const struct btp_hfp_disconnect_audio_cmd *cp = cmd;
	int err;

	LOG_INF("HFP disconnect audio");

	err = z_bt_hfp_disconnect_audio((const uint8_t *)&cp->address);
	if (err) {
		LOG_ERR("HFP disconnect audio failed: %d", err);
		return BTP_STATUS_FAILED;
	}

	return BTP_STATUS_SUCCESS;
}

static uint8_t hfp_terminate_call(const void *cmd, uint16_t cmd_len,
				  void *rsp, uint16_t *rsp_len)
{
	const struct btp_hfp_terminate_call_cmd *cp = cmd;
	int err;

	LOG_INF("HFP terminate call");

	err = z_bt_hfp_terminate_call((const uint8_t *)&cp->address);
	if (err) {
		LOG_ERR("HFP terminate call failed: %d", err);
		return BTP_STATUS_FAILED;
	}

	return BTP_STATUS_SUCCESS;
}

static uint8_t hfp_query_current_calls(const void *cmd, uint16_t cmd_len,
				       void *rsp, uint16_t *rsp_len)
{
	const struct btp_hfp_query_current_calls_cmd *cp = cmd;
	struct bt_conn *conn;
	int err;

	LOG_INF("HFP query current calls (AT+CLCC)");

	conn = bt_conn_lookup_addr_br(&cp->address.a);
	if (!conn) {
		LOG_ERR("No BR/EDR connection found");
		return BTP_STATUS_FAILED;
	}

	err = z_bt_hfp_hf_send_clcc(conn);
	bt_conn_unref(conn);

	if (err) {
		LOG_ERR("HFP query current calls failed: %d", err);
		return BTP_STATUS_FAILED;
	}

	return BTP_STATUS_SUCCESS;
}

static uint8_t hfp_get_subscriber_number(const void *cmd, uint16_t cmd_len,
					 void *rsp, uint16_t *rsp_len)
{
	const struct btp_hfp_get_subscriber_number_cmd *cp = cmd;
	int err;

	LOG_INF("HFP get subscriber number (AT+CNUM)");

	err = z_bt_hfp_get_subscriber_number((const uint8_t *)&cp->address);
	if (err) {
		LOG_ERR("HFP get subscriber number failed: %d", err);
		return BTP_STATUS_FAILED;
	}

	return BTP_STATUS_SUCCESS;
}

static uint8_t hfp_start_voice_recognition(const void *cmd, uint16_t cmd_len,
					   void *rsp, uint16_t *rsp_len)
{
	const struct btp_hfp_start_voice_recognition_cmd *cp = cmd;
	int err;

	LOG_INF("HFP start voice recognition");

	err = z_bt_hfp_start_voice_recognition((const uint8_t *)&cp->address);
	if (err) {
		LOG_ERR("HFP start VR failed: %d", err);
		return BTP_STATUS_FAILED;
	}

	return BTP_STATUS_SUCCESS;
}

static uint8_t hfp_stop_voice_recognition(const void *cmd, uint16_t cmd_len,
					  void *rsp, uint16_t *rsp_len)
{
	const struct btp_hfp_stop_voice_recognition_cmd *cp = cmd;
	int err;

	LOG_INF("HFP stop voice recognition");

	err = z_bt_hfp_stop_voice_recognition((const uint8_t *)&cp->address);
	if (err) {
		LOG_ERR("HFP stop VR failed: %d", err);
		return BTP_STATUS_FAILED;
	}

	return BTP_STATUS_SUCCESS;
}

static uint8_t hfp_call_control(const void *cmd, uint16_t cmd_len,
				void *rsp, uint16_t *rsp_len)
{
	const struct btp_hfp_call_control_cmd *cp = cmd;
	int err;

	LOG_INF("HFP call control: chld=%d index=%d", cp->chld, cp->index);

	err = z_bt_hfp_call_control((const uint8_t *)&cp->address,
				    cp->chld, cp->index);
	if (err) {
		LOG_ERR("HFP call control failed: %d", err);
		return BTP_STATUS_FAILED;
	}

	return BTP_STATUS_SUCCESS;
}

static uint8_t hfp_update_battery_level(const void *cmd, uint16_t cmd_len,
					void *rsp, uint16_t *rsp_len)
{
	const struct btp_hfp_update_battery_level_cmd *cp = cmd;
	int err;

	LOG_INF("HFP update battery level: %d", cp->level);

	err = z_bt_hfp_update_battery_level((const uint8_t *)&cp->address,
					    cp->level);
	if (err) {
		LOG_ERR("HFP update battery level failed: %d", err);
		return BTP_STATUS_FAILED;
	}

	return BTP_STATUS_SUCCESS;
}

/* Callbacks from z_api HFP layer */
void btp_hfp_connected_cb(const uint8_t *addr)
{
	struct btp_hfp_connected_ev ev;

	LOG_INF("HFP connected");

	memcpy(&ev.address, addr, sizeof(bt_addr_le_t));

	tester_event(BTP_SERVICE_ID_HFP, BTP_HFP_EV_CONNECTED,
		     &ev, sizeof(ev));
}

void btp_hfp_disconnected_cb(const uint8_t *addr)
{
	struct btp_hfp_disconnected_ev ev;

	LOG_INF("HFP disconnected");

	memcpy(&ev.address, addr, sizeof(bt_addr_le_t));

	tester_event(BTP_SERVICE_ID_HFP, BTP_HFP_EV_DISCONNECTED,
		     &ev, sizeof(ev));
}

void btp_hfp_audio_state_cb(const uint8_t *addr, uint8_t state)
{
	struct btp_hfp_audio_state_ev ev;

	LOG_INF("HFP audio state: %d", state);

	memcpy(&ev.address, addr, sizeof(bt_addr_le_t));
	ev.state = state;

	tester_event(BTP_SERVICE_ID_HFP, BTP_HFP_EV_AUDIO_STATE,
		     &ev, sizeof(ev));
}

static const struct btp_handler handlers[] = {
	{
		.opcode = BTP_HFP_READ_SUPPORTED_COMMANDS,
		.index = BTP_INDEX_NONE,
		.expect_len = 0,
		.func = supported_commands,
	},
	{
		.opcode = BTP_HFP_CONNECT,
		.index = BTP_INDEX,
		.expect_len = sizeof(struct btp_hfp_connect_cmd),
		.func = hfp_connect,
	},
	{
		.opcode = BTP_HFP_DISCONNECT,
		.index = BTP_INDEX,
		.expect_len = sizeof(struct btp_hfp_disconnect_cmd),
		.func = hfp_disconnect,
	},
	{
		.opcode = BTP_HFP_ANSWER_CALL,
		.index = BTP_INDEX,
		.expect_len = sizeof(struct btp_hfp_answer_call_cmd),
		.func = hfp_answer_call,
	},
	{
		.opcode = BTP_HFP_REJECT_CALL,
		.index = BTP_INDEX,
		.expect_len = sizeof(struct btp_hfp_reject_call_cmd),
		.func = hfp_reject_call,
	},
	{
		.opcode = BTP_HFP_DIAL,
		.index = BTP_INDEX,
		.expect_len = BTP_HANDLER_LENGTH_VARIABLE,
		.func = hfp_dial,
	},
	{
		.opcode = BTP_HFP_SET_VOLUME,
		.index = BTP_INDEX,
		.expect_len = sizeof(struct btp_hfp_set_volume_cmd),
		.func = hfp_set_volume,
	},
	{
		.opcode = BTP_HFP_SEND_DTMF,
		.index = BTP_INDEX,
		.expect_len = sizeof(struct btp_hfp_send_dtmf_cmd),
		.func = hfp_send_dtmf,
	},
	{
		.opcode = BTP_HFP_CONNECT_ACL,
		.index = BTP_INDEX,
		.expect_len = sizeof(struct btp_hfp_connect_acl_cmd),
		.func = hfp_connect_acl,
	},
	{
		.opcode = BTP_HFP_CONNECT_AUDIO,
		.index = BTP_INDEX,
		.expect_len = sizeof(struct btp_hfp_connect_audio_cmd),
		.func = hfp_connect_audio,
	},
	{
		.opcode = BTP_HFP_DISCONNECT_AUDIO,
		.index = BTP_INDEX,
		.expect_len = sizeof(struct btp_hfp_disconnect_audio_cmd),
		.func = hfp_disconnect_audio,
	},
	{
		.opcode = BTP_HFP_TERMINATE_CALL,
		.index = BTP_INDEX,
		.expect_len = sizeof(struct btp_hfp_terminate_call_cmd),
		.func = hfp_terminate_call,
	},
	{
		.opcode = BTP_HFP_QUERY_CURRENT_CALLS,
		.index = BTP_INDEX,
		.expect_len = sizeof(struct btp_hfp_query_current_calls_cmd),
		.func = hfp_query_current_calls,
	},
	{
		.opcode = BTP_HFP_GET_SUBSCRIBER_NUMBER,
		.index = BTP_INDEX,
		.expect_len = sizeof(struct btp_hfp_get_subscriber_number_cmd),
		.func = hfp_get_subscriber_number,
	},
	{
		.opcode = BTP_HFP_START_VOICE_RECOGNITION,
		.index = BTP_INDEX,
		.expect_len = sizeof(struct btp_hfp_start_voice_recognition_cmd),
		.func = hfp_start_voice_recognition,
	},
	{
		.opcode = BTP_HFP_STOP_VOICE_RECOGNITION,
		.index = BTP_INDEX,
		.expect_len = sizeof(struct btp_hfp_stop_voice_recognition_cmd),
		.func = hfp_stop_voice_recognition,
	},
	{
		.opcode = BTP_HFP_CALL_CONTROL,
		.index = BTP_INDEX,
		.expect_len = sizeof(struct btp_hfp_call_control_cmd),
		.func = hfp_call_control,
	},
	{
		.opcode = BTP_HFP_UPDATE_BATTERY_LEVEL,
		.index = BTP_INDEX,
		.expect_len = sizeof(struct btp_hfp_update_battery_level_cmd),
		.func = hfp_update_battery_level,
	},
};

uint8_t tester_init_hfp(void)
{
	tester_register_command_handlers(BTP_SERVICE_ID_HFP, handlers,
					ARRAY_SIZE(handlers));

	/* Register HFP callbacks early so that incoming connections
	 * from PTS (IUT as acceptor) are properly handled.
	 * hfp_init_in_ipc has a guard to prevent double registration.
	 */
	z_bt_hfp_init();

	LOG_INF("HFP service initialized");

	return BTP_STATUS_SUCCESS;
}

uint8_t tester_unregister_hfp(void)
{
	LOG_INF("HFP service unregistered");

	return BTP_STATUS_SUCCESS;
}

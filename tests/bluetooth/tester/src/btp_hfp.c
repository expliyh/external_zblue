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

/* ================================================================
 * HFP AG BTP handlers
 * ================================================================ */

#include "btp/btp_hfp_ag.h"

/* Forward declarations for z_api HFP AG functions */
extern int z_bt_hfp_ag_init(void);
extern int z_bt_hfp_connect_acl(const uint8_t *addr);
extern int z_bt_hfp_ag_slc_connect(const uint8_t *addr);
extern int z_bt_hfp_ag_slc_disconnect(const uint8_t *addr);
extern int z_bt_hfp_ag_connect_audio(const uint8_t *addr);
extern int z_bt_hfp_ag_disconnect_audio(const uint8_t *addr);
extern int z_bt_hfp_ag_start_virtual_call(const uint8_t *addr);
extern int z_bt_hfp_ag_stop_virtual_call(const uint8_t *addr);
extern int z_bt_hfp_ag_phone_state_change(const uint8_t *addr,
	uint8_t num_active, uint8_t num_held,
	uint8_t call_state, uint8_t addr_type,
	const char *number, const char *name);
extern int z_bt_hfp_ag_volume_control(const uint8_t *addr,
	uint8_t type, uint8_t volume);
extern int z_bt_hfp_ag_start_voice_recognition(const uint8_t *addr);
extern int z_bt_hfp_ag_stop_voice_recognition(const uint8_t *addr);
extern int z_bt_hfp_ag_device_status(const uint8_t *addr,
	uint8_t network, uint8_t roam, uint8_t signal, uint8_t battery);
extern int z_bt_hfp_ag_send_at_cmd(const uint8_t *addr, const char *cmd);
extern int z_bt_hfp_ag_clcc_response(const uint8_t *addr, uint32_t index,
	uint8_t dir, uint8_t call_state, uint8_t mode, uint8_t mpty,
	uint8_t addr_type, const char *number);
extern int z_bt_hfp_ag_cind_response(const uint8_t *addr,
	uint8_t network, uint8_t call, uint8_t callsetup,
	uint8_t callheld, uint8_t signal, uint8_t roam, uint8_t battery);
extern int z_bt_hfp_ag_dial_response(uint8_t result);

static uint8_t ag_supported_commands(const void *cmd, uint16_t cmd_len,
				     void *rsp, uint16_t *rsp_len)
{
	struct btp_hfp_ag_read_supported_commands_rp *rp = rsp;

	memset(rp->data, 0, 3);
	tester_set_bit(rp->data, BTP_HFP_AG_READ_SUPPORTED_COMMANDS);
	tester_set_bit(rp->data, BTP_HFP_AG_CONNECT);
	tester_set_bit(rp->data, BTP_HFP_AG_DISCONNECT);
	tester_set_bit(rp->data, BTP_HFP_AG_CONNECT_AUDIO);
	tester_set_bit(rp->data, BTP_HFP_AG_DISCONNECT_AUDIO);
	tester_set_bit(rp->data, BTP_HFP_AG_PHONE_STATE_CHANGE);
	tester_set_bit(rp->data, BTP_HFP_AG_VOLUME_CONTROL);
	tester_set_bit(rp->data, BTP_HFP_AG_START_VR);
	tester_set_bit(rp->data, BTP_HFP_AG_STOP_VR);
	tester_set_bit(rp->data, BTP_HFP_AG_DEVICE_STATUS);
	tester_set_bit(rp->data, BTP_HFP_AG_CLCC_RESPONSE);
	tester_set_bit(rp->data, BTP_HFP_AG_CIND_RESPONSE);
	tester_set_bit(rp->data, BTP_HFP_AG_DIAL_RESPONSE);
	tester_set_bit(rp->data, BTP_HFP_AG_SEND_AT_CMD);
	tester_set_bit(rp->data, BTP_HFP_AG_START_VIRTUAL_CALL);
	tester_set_bit(rp->data, BTP_HFP_AG_STOP_VIRTUAL_CALL);

	*rsp_len = 3;
	return BTP_STATUS_SUCCESS;
}

static uint8_t ag_connect(const void *cmd, uint16_t cmd_len,
			  void *rsp, uint16_t *rsp_len)
{
	const struct btp_hfp_ag_connect_cmd *cp = cmd;

	LOG_INF("HFP AG connect");
	if (z_bt_hfp_ag_slc_connect((const uint8_t *)&cp->address))
		return BTP_STATUS_FAILED;
	return BTP_STATUS_SUCCESS;
}

static uint8_t ag_disconnect(const void *cmd, uint16_t cmd_len,
			     void *rsp, uint16_t *rsp_len)
{
	const struct btp_hfp_ag_disconnect_cmd *cp = cmd;

	LOG_INF("HFP AG disconnect");
	if (z_bt_hfp_ag_slc_disconnect((const uint8_t *)&cp->address))
		return BTP_STATUS_FAILED;
	return BTP_STATUS_SUCCESS;
}

static uint8_t ag_connect_audio(const void *cmd, uint16_t cmd_len,
				void *rsp, uint16_t *rsp_len)
{
	const struct btp_hfp_ag_connect_audio_cmd *cp = cmd;

	LOG_INF("HFP AG connect audio");
	if (z_bt_hfp_ag_connect_audio((const uint8_t *)&cp->address))
		return BTP_STATUS_FAILED;
	return BTP_STATUS_SUCCESS;
}

static uint8_t ag_disconnect_audio(const void *cmd, uint16_t cmd_len,
				   void *rsp, uint16_t *rsp_len)
{
	const struct btp_hfp_ag_disconnect_audio_cmd *cp = cmd;

	LOG_INF("HFP AG disconnect audio");
	if (z_bt_hfp_ag_disconnect_audio((const uint8_t *)&cp->address))
		return BTP_STATUS_FAILED;
	return BTP_STATUS_SUCCESS;
}

static uint8_t ag_phone_state_change(const void *cmd, uint16_t cmd_len,
				     void *rsp, uint16_t *rsp_len)
{
	const struct btp_hfp_ag_phone_state_change_cmd *cp = cmd;
	char number[81] = {0};

	LOG_INF("HFP AG phone state change");

	if (cp->number_len > 0 && cp->number_len <= 80)
		memcpy(number, cp->number, cp->number_len);

	if (z_bt_hfp_ag_phone_state_change((const uint8_t *)&cp->address,
		cp->num_active, cp->num_held, cp->call_state,
		cp->addr_type, number[0] ? number : NULL, NULL))
		return BTP_STATUS_FAILED;
	return BTP_STATUS_SUCCESS;
}

static uint8_t ag_volume_control(const void *cmd, uint16_t cmd_len,
				 void *rsp, uint16_t *rsp_len)
{
	const struct btp_hfp_ag_volume_control_cmd *cp = cmd;

	LOG_INF("HFP AG volume control");
	if (z_bt_hfp_ag_volume_control((const uint8_t *)&cp->address,
		cp->type, cp->volume))
		return BTP_STATUS_FAILED;
	return BTP_STATUS_SUCCESS;
}

static uint8_t ag_start_vr(const void *cmd, uint16_t cmd_len,
			   void *rsp, uint16_t *rsp_len)
{
	const struct btp_hfp_ag_start_vr_cmd *cp = cmd;

	LOG_INF("HFP AG start VR");
	if (z_bt_hfp_ag_start_voice_recognition((const uint8_t *)&cp->address))
		return BTP_STATUS_FAILED;
	return BTP_STATUS_SUCCESS;
}

static uint8_t ag_stop_vr(const void *cmd, uint16_t cmd_len,
			  void *rsp, uint16_t *rsp_len)
{
	const struct btp_hfp_ag_stop_vr_cmd *cp = cmd;

	LOG_INF("HFP AG stop VR");
	if (z_bt_hfp_ag_stop_voice_recognition((const uint8_t *)&cp->address))
		return BTP_STATUS_FAILED;
	return BTP_STATUS_SUCCESS;
}

static uint8_t ag_device_status(const void *cmd, uint16_t cmd_len,
				void *rsp, uint16_t *rsp_len)
{
	const struct btp_hfp_ag_device_status_cmd *cp = cmd;

	LOG_INF("HFP AG device status");
	if (z_bt_hfp_ag_device_status((const uint8_t *)&cp->address,
		cp->network, cp->roam, cp->signal, cp->battery))
		return BTP_STATUS_FAILED;
	return BTP_STATUS_SUCCESS;
}

static uint8_t ag_clcc_response(const void *cmd, uint16_t cmd_len,
				void *rsp, uint16_t *rsp_len)
{
	const struct btp_hfp_ag_clcc_response_cmd *cp = cmd;
	char number[81] = {0};

	if (cp->number_len > 0 && cp->number_len <= 80)
		memcpy(number, cp->number, cp->number_len);

	if (z_bt_hfp_ag_clcc_response((const uint8_t *)&cp->address,
		cp->index, cp->dir, cp->call_state, cp->mode, cp->mpty,
		cp->addr_type, number[0] ? number : NULL))
		return BTP_STATUS_FAILED;
	return BTP_STATUS_SUCCESS;
}

static uint8_t ag_cind_response(const void *cmd, uint16_t cmd_len,
				void *rsp, uint16_t *rsp_len)
{
	const struct btp_hfp_ag_cind_response_cmd *cp = cmd;

	if (z_bt_hfp_ag_cind_response((const uint8_t *)&cp->address,
		cp->network, cp->call, cp->callsetup, cp->callheld,
		cp->signal, cp->roam, cp->battery))
		return BTP_STATUS_FAILED;
	return BTP_STATUS_SUCCESS;
}

static uint8_t ag_dial_response(const void *cmd, uint16_t cmd_len,
				void *rsp, uint16_t *rsp_len)
{
	const struct btp_hfp_ag_dial_response_cmd *cp = cmd;

	if (z_bt_hfp_ag_dial_response(cp->result))
		return BTP_STATUS_FAILED;
	return BTP_STATUS_SUCCESS;
}

extern int z_bt_hfp_ag_redial_response(uint8_t result, const char *number);

static uint8_t ag_redial_response(const void *cmd, uint16_t cmd_len,
				  void *rsp, uint16_t *rsp_len)
{
	const struct btp_hfp_ag_redial_response_cmd *cp = cmd;
	char number[81] = {0};
	uint16_t len = cp->number_len;

	if (len > 0) {
		if (len >= sizeof(number))
			len = sizeof(number) - 1;
		memcpy(number, cp->number, len);
		number[len] = '\0';
	}

	if (z_bt_hfp_ag_redial_response(cp->result, len > 0 ? number : NULL))
		return BTP_STATUS_FAILED;
	return BTP_STATUS_SUCCESS;
}

static uint8_t ag_send_at_cmd(const void *cmd, uint16_t cmd_len,
			      void *rsp, uint16_t *rsp_len)
{
	const struct btp_hfp_ag_send_at_cmd_cmd *cp = cmd;
	char at_cmd[256] = {0};

	if (cp->cmd_len > 0 && cp->cmd_len < 256)
		memcpy(at_cmd, cp->cmd, cp->cmd_len);

	LOG_INF("HFP AG send AT cmd: %s", at_cmd);
	if (z_bt_hfp_ag_send_at_cmd((const uint8_t *)&cp->address, at_cmd))
		return BTP_STATUS_FAILED;
	return BTP_STATUS_SUCCESS;
}

static uint8_t ag_start_virtual_call(const void *cmd, uint16_t cmd_len,
				     void *rsp, uint16_t *rsp_len)
{
	const struct btp_hfp_ag_start_virtual_call_cmd *cp = cmd;

	LOG_INF("HFP AG start virtual call");
	if (z_bt_hfp_ag_start_virtual_call((const uint8_t *)&cp->address))
		return BTP_STATUS_FAILED;
	return BTP_STATUS_SUCCESS;
}

static uint8_t ag_stop_virtual_call(const void *cmd, uint16_t cmd_len,
				    void *rsp, uint16_t *rsp_len)
{
	const struct btp_hfp_ag_stop_virtual_call_cmd *cp = cmd;

	LOG_INF("HFP AG stop virtual call");
	if (z_bt_hfp_ag_stop_virtual_call((const uint8_t *)&cp->address))
		return BTP_STATUS_FAILED;
	return BTP_STATUS_SUCCESS;
}

/* AG BTP event callbacks (called from z_api_hfp_ag.c) */

void btp_hfp_ag_connected_cb(const uint8_t *addr)
{
	struct btp_hfp_ag_connected_ev ev;

	LOG_INF("HFP AG connected");
	memcpy(&ev.address, addr, sizeof(bt_addr_le_t));
	tester_event(BTP_SERVICE_ID_HFP_AG, BTP_HFP_AG_EV_CONNECTED,
		     &ev, sizeof(ev));
}

void btp_hfp_ag_disconnected_cb(const uint8_t *addr)
{
	struct btp_hfp_ag_disconnected_ev ev;

	LOG_INF("HFP AG disconnected");
	memcpy(&ev.address, addr, sizeof(bt_addr_le_t));
	tester_event(BTP_SERVICE_ID_HFP_AG, BTP_HFP_AG_EV_DISCONNECTED,
		     &ev, sizeof(ev));
}

void btp_hfp_ag_audio_state_cb(const uint8_t *addr, uint8_t state)
{
	struct btp_hfp_ag_audio_state_ev ev;

	LOG_INF("HFP AG audio state: %d", state);
	memcpy(&ev.address, addr, sizeof(bt_addr_le_t));
	ev.state = state;
	tester_event(BTP_SERVICE_ID_HFP_AG, BTP_HFP_AG_EV_AUDIO_STATE,
		     &ev, sizeof(ev));
}

void btp_hfp_ag_vr_state_cb(const uint8_t *addr, bool started)
{
	struct btp_hfp_ag_vr_state_ev ev;

	LOG_INF("HFP AG VR state: %d", started);
	memcpy(&ev.address, addr, sizeof(bt_addr_le_t));
	ev.started = started ? 1 : 0;
	tester_event(BTP_SERVICE_ID_HFP_AG, BTP_HFP_AG_EV_VR_STATE,
		     &ev, sizeof(ev));
}

void btp_hfp_ag_battery_update_cb(const uint8_t *addr, uint8_t level)
{
	struct btp_hfp_ag_battery_update_ev ev;

	LOG_INF("HFP AG battery update: %d", level);
	memcpy(&ev.address, addr, sizeof(bt_addr_le_t));
	ev.level = level;
	tester_event(BTP_SERVICE_ID_HFP_AG, BTP_HFP_AG_EV_BATTERY_UPDATE,
		     &ev, sizeof(ev));
}

void btp_hfp_ag_volume_cb(const uint8_t *addr, uint8_t type, uint8_t vol)
{
	struct btp_hfp_ag_volume_control_ev ev;

	LOG_INF("HFP AG volume: type=%d vol=%d", type, vol);
	memcpy(&ev.address, addr, sizeof(bt_addr_le_t));
	ev.type = type;
	ev.volume = vol;
	tester_event(BTP_SERVICE_ID_HFP_AG, BTP_HFP_AG_EV_VOLUME_CONTROL,
		     &ev, sizeof(ev));
}

void btp_hfp_ag_answer_call_cb(const uint8_t *addr)
{
	struct btp_hfp_ag_answer_call_ev ev;

	LOG_INF("HFP AG answer call");
	memcpy(&ev.address, addr, sizeof(bt_addr_le_t));
	tester_event(BTP_SERVICE_ID_HFP_AG, BTP_HFP_AG_EV_ANSWER_CALL,
		     &ev, sizeof(ev));
}

void btp_hfp_ag_reject_call_cb(const uint8_t *addr)
{
	struct btp_hfp_ag_reject_call_ev ev;

	LOG_INF("HFP AG reject call");
	memcpy(&ev.address, addr, sizeof(bt_addr_le_t));
	tester_event(BTP_SERVICE_ID_HFP_AG, BTP_HFP_AG_EV_REJECT_CALL,
		     &ev, sizeof(ev));
}

void btp_hfp_ag_hangup_call_cb(const uint8_t *addr)
{
	struct btp_hfp_ag_hangup_call_ev ev;

	LOG_INF("HFP AG hangup call");
	memcpy(&ev.address, addr, sizeof(bt_addr_le_t));
	tester_event(BTP_SERVICE_ID_HFP_AG, BTP_HFP_AG_EV_HANGUP_CALL,
		     &ev, sizeof(ev));
}

void btp_hfp_ag_dial_cb(const uint8_t *addr, const char *number)
{
	uint8_t buf[sizeof(struct btp_hfp_ag_dial_ev) + 80];
	struct btp_hfp_ag_dial_ev *ev = (void *)buf;
	uint8_t num_len = 0;

	LOG_INF("HFP AG dial: %s", number ? number : "(redial)");

	memcpy(&ev->address, addr, sizeof(bt_addr_le_t));
	if (number) {
		num_len = strlen(number);
		if (num_len > 80)
			num_len = 80;
		memcpy(ev->number, number, num_len);
	}
	ev->number_len = num_len;
	tester_event(BTP_SERVICE_ID_HFP_AG, BTP_HFP_AG_EV_DIAL,
		     buf, sizeof(struct btp_hfp_ag_dial_ev) + num_len);
}

void btp_hfp_ag_at_cmd_cb(const uint8_t *addr, const char *cmd)
{
	uint8_t buf[sizeof(struct btp_hfp_ag_at_cmd_ev) + 255];
	struct btp_hfp_ag_at_cmd_ev *ev = (void *)buf;
	uint8_t cmd_len = 0;

	LOG_INF("HFP AG AT cmd: %s", cmd ? cmd : "");
	memcpy(&ev->address, addr, sizeof(bt_addr_le_t));
	if (cmd) {
		cmd_len = strlen(cmd);
		if (cmd_len > 255)
			cmd_len = 255;
		memcpy(ev->cmd, cmd, cmd_len);
	}
	ev->cmd_len = cmd_len;
	tester_event(BTP_SERVICE_ID_HFP_AG, BTP_HFP_AG_EV_AT_CMD,
		     buf, sizeof(struct btp_hfp_ag_at_cmd_ev) + cmd_len);
}

void btp_hfp_ag_clcc_request_cb(const uint8_t *addr)
{
	struct btp_hfp_ag_clcc_request_ev ev;

	LOG_INF("HFP AG CLCC request");
	memcpy(&ev.address, addr, sizeof(bt_addr_le_t));
	tester_event(BTP_SERVICE_ID_HFP_AG, BTP_HFP_AG_EV_CLCC_REQUEST,
		     &ev, sizeof(ev));
}

void btp_hfp_ag_cind_request_cb(const uint8_t *addr)
{
	struct btp_hfp_ag_cind_request_ev ev;

	LOG_INF("HFP AG CIND request");
	memcpy(&ev.address, addr, sizeof(bt_addr_le_t));
	tester_event(BTP_SERVICE_ID_HFP_AG, BTP_HFP_AG_EV_CIND_REQUEST,
		     &ev, sizeof(ev));
}

void btp_hfp_ag_call_control_cb(const uint8_t *addr, uint8_t chld)
{
	struct btp_hfp_ag_call_control_ev ev;

	LOG_INF("HFP AG call control (CHLD): %u", chld);
	memcpy(&ev.address, addr, sizeof(bt_addr_le_t));
	ev.chld = chld;
	tester_event(BTP_SERVICE_ID_HFP_AG, BTP_HFP_AG_EV_CALL_CONTROL,
		     &ev, sizeof(ev));
}

void btp_hfp_ag_dtmf_cb(const uint8_t *addr, uint8_t code)
{
	struct btp_hfp_ag_dtmf_ev ev;

	LOG_INF("HFP AG DTMF: 0x%02x", code);
	memcpy(&ev.address, addr, sizeof(bt_addr_le_t));
	ev.code = code;
	tester_event(BTP_SERVICE_ID_HFP_AG, BTP_HFP_AG_EV_DTMF,
		     &ev, sizeof(ev));
}

void btp_hfp_ag_nrec_cb(const uint8_t *addr, uint8_t enable)
{
	struct btp_hfp_ag_nrec_ev ev;

	LOG_INF("HFP AG NREC: %u", enable);
	memcpy(&ev.address, addr, sizeof(bt_addr_le_t));
	ev.enable = enable;
	tester_event(BTP_SERVICE_ID_HFP_AG, BTP_HFP_AG_EV_NREC,
		     &ev, sizeof(ev));
}

void btp_hfp_ag_cops_request_cb(const uint8_t *addr)
{
	struct btp_hfp_ag_cops_request_ev ev;

	LOG_INF("HFP AG COPS request");
	memcpy(&ev.address, addr, sizeof(bt_addr_le_t));
	tester_event(BTP_SERVICE_ID_HFP_AG, BTP_HFP_AG_EV_COPS_REQUEST,
		     &ev, sizeof(ev));
}

const char *btp_hfp_ag_redial_cb(const uint8_t *addr, char *buf, size_t len)
{
	/* Unused: replaced by async btp_hfp_ag_redial_req_cb + redial_response. */
	(void)addr; (void)buf; (void)len;
	return NULL;
}

void btp_hfp_ag_redial_req_cb(const uint8_t *addr)
{
	struct btp_hfp_ag_redial_req_ev ev;

	LOG_INF("HFP AG redial request (AT+BLDN)");
	memcpy(&ev.address, addr, sizeof(bt_addr_le_t));
	tester_event(BTP_SERVICE_ID_HFP_AG, BTP_HFP_AG_EV_REDIAL_REQ,
		     &ev, sizeof(ev));
}

static uint8_t ag_connect_acl(const void *cmd, uint16_t cmd_len,
			      void *rsp, uint16_t *rsp_len)
{
	const struct btp_hfp_ag_connect_acl_cmd *cp = cmd;

	LOG_INF("HFP AG connect ACL only");
	if (z_bt_hfp_connect_acl((const uint8_t *)&cp->address))
		return BTP_STATUS_FAILED;
	return BTP_STATUS_SUCCESS;
}

static const struct btp_handler ag_handlers[] = {
	{
		.opcode = BTP_HFP_AG_READ_SUPPORTED_COMMANDS,
		.index = BTP_INDEX_NONE,
		.expect_len = 0,
		.func = ag_supported_commands,
	},
	{
		.opcode = BTP_HFP_AG_CONNECT,
		.index = BTP_INDEX,
		.expect_len = sizeof(struct btp_hfp_ag_connect_cmd),
		.func = ag_connect,
	},
	{
		.opcode = BTP_HFP_AG_DISCONNECT,
		.index = BTP_INDEX,
		.expect_len = sizeof(struct btp_hfp_ag_disconnect_cmd),
		.func = ag_disconnect,
	},
	{
		.opcode = BTP_HFP_AG_CONNECT_AUDIO,
		.index = BTP_INDEX,
		.expect_len = sizeof(struct btp_hfp_ag_connect_audio_cmd),
		.func = ag_connect_audio,
	},
	{
		.opcode = BTP_HFP_AG_DISCONNECT_AUDIO,
		.index = BTP_INDEX,
		.expect_len = sizeof(struct btp_hfp_ag_disconnect_audio_cmd),
		.func = ag_disconnect_audio,
	},
	{
		.opcode = BTP_HFP_AG_PHONE_STATE_CHANGE,
		.index = BTP_INDEX,
		.expect_len = BTP_HANDLER_LENGTH_VARIABLE,
		.func = ag_phone_state_change,
	},
	{
		.opcode = BTP_HFP_AG_VOLUME_CONTROL,
		.index = BTP_INDEX,
		.expect_len = sizeof(struct btp_hfp_ag_volume_control_cmd),
		.func = ag_volume_control,
	},
	{
		.opcode = BTP_HFP_AG_START_VR,
		.index = BTP_INDEX,
		.expect_len = sizeof(struct btp_hfp_ag_start_vr_cmd),
		.func = ag_start_vr,
	},
	{
		.opcode = BTP_HFP_AG_STOP_VR,
		.index = BTP_INDEX,
		.expect_len = sizeof(struct btp_hfp_ag_stop_vr_cmd),
		.func = ag_stop_vr,
	},
	{
		.opcode = BTP_HFP_AG_DEVICE_STATUS,
		.index = BTP_INDEX,
		.expect_len = sizeof(struct btp_hfp_ag_device_status_cmd),
		.func = ag_device_status,
	},
	{
		.opcode = BTP_HFP_AG_CLCC_RESPONSE,
		.index = BTP_INDEX,
		.expect_len = BTP_HANDLER_LENGTH_VARIABLE,
		.func = ag_clcc_response,
	},
	{
		.opcode = BTP_HFP_AG_CIND_RESPONSE,
		.index = BTP_INDEX,
		.expect_len = sizeof(struct btp_hfp_ag_cind_response_cmd),
		.func = ag_cind_response,
	},
	{
		.opcode = BTP_HFP_AG_DIAL_RESPONSE,
		.index = BTP_INDEX,
		.expect_len = sizeof(struct btp_hfp_ag_dial_response_cmd),
		.func = ag_dial_response,
	},
	{
		.opcode = BTP_HFP_AG_SEND_AT_CMD,
		.index = BTP_INDEX,
		.expect_len = BTP_HANDLER_LENGTH_VARIABLE,
		.func = ag_send_at_cmd,
	},
	{
		.opcode = BTP_HFP_AG_START_VIRTUAL_CALL,
		.index = BTP_INDEX,
		.expect_len = sizeof(struct btp_hfp_ag_start_virtual_call_cmd),
		.func = ag_start_virtual_call,
	},
	{
		.opcode = BTP_HFP_AG_STOP_VIRTUAL_CALL,
		.index = BTP_INDEX,
		.expect_len = sizeof(struct btp_hfp_ag_stop_virtual_call_cmd),
		.func = ag_stop_virtual_call,
	},
	{
		.opcode = BTP_HFP_AG_CONNECT_ACL,
		.index = BTP_INDEX,
		.expect_len = sizeof(struct btp_hfp_ag_connect_acl_cmd),
		.func = ag_connect_acl,
	},
	{
		.opcode = BTP_HFP_AG_REDIAL_RESPONSE,
		.index = BTP_INDEX,
		.expect_len = BTP_HANDLER_LENGTH_VARIABLE,
		.func = ag_redial_response,
	},
};

uint8_t tester_init_hfp_ag(void)
{
	tester_register_command_handlers(BTP_SERVICE_ID_HFP_AG, ag_handlers,
					ARRAY_SIZE(ag_handlers));

	z_bt_hfp_ag_init();

	LOG_INF("HFP AG service initialized");
	return BTP_STATUS_SUCCESS;
}

uint8_t tester_unregister_hfp_ag(void)
{
	LOG_INF("HFP AG service unregistered");
	return BTP_STATUS_SUCCESS;
}

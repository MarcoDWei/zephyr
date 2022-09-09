/*
 * Copyright (c) 2022 Nordic Semiconductor ASA
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdlib.h>
#include <zephyr/shell/shell.h>
#include <zephyr/bluetooth/mesh.h>

#include "mesh/net.h"
#include "mesh/access.h"
#include "utils.h"
#include <zephyr/bluetooth/mesh/shell.h>

static struct bt_mesh_model *mod;

static void show_faults(const struct shell *sh, uint8_t test_id, uint16_t cid, uint8_t *faults,
			size_t fault_count)
{
	size_t i;

	if (!fault_count) {
		shell_print(sh, "Health Test ID 0x%02x Company ID 0x%04x: no faults\n", test_id,
			    cid);
		return;
	}

	shell_print(sh, "Health Test ID 0x%02x Company ID 0x%04x Fault Count %zu:\n", test_id, cid,
		    fault_count);

	for (i = 0; i < fault_count; i++) {
		shell_print(sh, "\t0x%02x\n", faults[i]);
	}
}

static int cmd_fault_get(const struct shell *sh, size_t argc, char *argv[])
{
	if (!mod && !bt_mesh_shell_mdl_first_get(BT_MESH_MODEL_ID_HEALTH_CLI, &mod)) {
		return -ENODEV;
	}

	struct bt_mesh_health_cli *cli = mod->user_data;
	struct bt_mesh_msg_ctx ctx = {
		.net_idx = bt_mesh_shell_target_ctx.net_idx,
		.addr = bt_mesh_shell_target_ctx.dst,
		.app_idx = bt_mesh_shell_target_ctx.app_idx,
	};

	uint8_t faults[32];
	size_t fault_count;
	uint8_t test_id;
	uint16_t cid;
	int err = 0;

	cid = shell_strtoul(argv[1], 0, &err);
	if (err) {
		shell_warn(sh, "Unable to parse input string argument");
		return err;
	}

	fault_count = sizeof(faults);

	err = bt_mesh_health_cli_fault_get(cli, ctx.addr ? &ctx : NULL, cid, &test_id, faults,
					   &fault_count);
	if (err) {
		shell_error(sh, "Failed to send Health Fault Get (err %d)", err);
	} else {
		show_faults(sh, test_id, cid, faults, fault_count);
	}

	return 0;
}

static int fault_clear(const struct shell *sh, size_t argc, char *argv[], bool acked)
{
	if (!mod && !bt_mesh_shell_mdl_first_get(BT_MESH_MODEL_ID_HEALTH_CLI, &mod)) {
		return -ENODEV;
	}

	struct bt_mesh_health_cli *cli = mod->user_data;
	struct bt_mesh_msg_ctx ctx = {
		.net_idx = bt_mesh_shell_target_ctx.net_idx,
		.addr = bt_mesh_shell_target_ctx.dst,
		.app_idx = bt_mesh_shell_target_ctx.app_idx,
	};

	uint8_t test_id;
	uint16_t cid;
	int err = 0;

	cid = shell_strtoul(argv[1], 0, &err);
	if (err) {
		shell_warn(sh, "Unable to parse input string argument");
		return err;
	}

	if (acked) {
		uint8_t faults[32];
		size_t fault_count = sizeof(faults);

		err = bt_mesh_health_cli_fault_clear(cli, ctx.addr ? &ctx : NULL, cid, &test_id,
						     faults, &fault_count);
		if (err) {
			shell_error(sh, "Failed to send Health Fault Clear (err %d)", err);
		} else {
			show_faults(sh, test_id, cid, faults, fault_count);
		}

		return err;
	}

	err = bt_mesh_health_cli_fault_clear_unack(cli, ctx.addr ? &ctx : NULL, cid);
	if (err) {
		shell_error(sh, "Health Fault Clear Unacknowledged failed (err %d)", err);
	}

	return err;
}

static int cmd_fault_clear(const struct shell *sh, size_t argc, char *argv[])
{
	return fault_clear(sh, argc, argv, true);
}

static int cmd_fault_clear_unack(const struct shell *sh, size_t argc, char *argv[])
{
	return fault_clear(sh, argc, argv, false);
}

static int fault_test(const struct shell *sh, size_t argc, char *argv[], bool acked)
{
	if (!mod && !bt_mesh_shell_mdl_first_get(BT_MESH_MODEL_ID_HEALTH_CLI, &mod)) {
		return -ENODEV;
	}

	struct bt_mesh_health_cli *cli = mod->user_data;
	struct bt_mesh_msg_ctx ctx = {
		.net_idx = bt_mesh_shell_target_ctx.net_idx,
		.addr = bt_mesh_shell_target_ctx.dst,
		.app_idx = bt_mesh_shell_target_ctx.app_idx,
	};

	uint8_t test_id;
	uint16_t cid;
	int err = 0;

	cid = shell_strtoul(argv[1], 0, &err);
	test_id = shell_strtoul(argv[2], 0, &err);
	if (err) {
		shell_warn(sh, "Unable to parse input string argument");
		return err;
	}

	if (acked) {
		uint8_t faults[32];
		size_t fault_count = sizeof(faults);

		err = bt_mesh_health_cli_fault_test(cli, ctx.addr ? &ctx : NULL, cid, test_id,
						    faults, &fault_count);
		if (err) {
			shell_error(sh, "Failed to send Health Fault Test (err %d)", err);
		} else {
			show_faults(sh, test_id, cid, faults, fault_count);
		}

		return err;
	}

	err = bt_mesh_health_cli_fault_test_unack(cli, ctx.addr ? &ctx : NULL, cid, test_id);
	if (err) {
		shell_error(sh, "Health Fault Test Unacknowledged failed (err %d)", err);
	}

	return err;
}

static int cmd_fault_test(const struct shell *sh, size_t argc, char *argv[])
{
	return fault_test(sh, argc, argv, true);
}

static int cmd_fault_test_unack(const struct shell *sh, size_t argc, char *argv[])
{
	return fault_test(sh, argc, argv, false);
}

static int cmd_period_get(const struct shell *sh, size_t argc, char *argv[])
{
	if (!mod && !bt_mesh_shell_mdl_first_get(BT_MESH_MODEL_ID_HEALTH_CLI, &mod)) {
		return -ENODEV;
	}

	struct bt_mesh_health_cli *cli = mod->user_data;
	struct bt_mesh_msg_ctx ctx = {
		.net_idx = bt_mesh_shell_target_ctx.net_idx,
		.addr = bt_mesh_shell_target_ctx.dst,
		.app_idx = bt_mesh_shell_target_ctx.app_idx,
	};

	uint8_t divisor;
	int err;

	err = bt_mesh_health_cli_period_get(cli, ctx.addr ? &ctx : NULL, &divisor);
	if (err) {
		shell_error(sh, "Failed to send Health Period Get (err %d)", err);
	} else {
		shell_print(sh, "Health FastPeriodDivisor: %u", divisor);
	}

	return 0;
}

static int period_set(const struct shell *sh, size_t argc, char *argv[], bool acked)
{
	if (!mod && !bt_mesh_shell_mdl_first_get(BT_MESH_MODEL_ID_HEALTH_CLI, &mod)) {
		return -ENODEV;
	}

	struct bt_mesh_health_cli *cli = mod->user_data;
	struct bt_mesh_msg_ctx ctx = {
		.net_idx = bt_mesh_shell_target_ctx.net_idx,
		.addr = bt_mesh_shell_target_ctx.dst,
		.app_idx = bt_mesh_shell_target_ctx.app_idx,
	};

	uint8_t divisor;
	int err = 0;

	divisor = shell_strtoul(argv[1], 0, &err);
	if (err) {
		shell_warn(sh, "Unable to parse input string argument");
		return err;
	}

	if (acked) {
		uint8_t updated_divisor;

		err = bt_mesh_health_cli_period_set(cli, ctx.addr ? &ctx : NULL, divisor,
						    &updated_divisor);
		if (err) {
			shell_error(sh, "Failed to send Health Period Set (err %d)", err);
		} else {
			shell_print(sh, "Health FastPeriodDivisor: %u", updated_divisor);
		}

		return err;
	}

	err = bt_mesh_health_cli_period_set_unack(cli, ctx.addr ? &ctx : NULL, divisor);
	if (err) {
		shell_print(sh, "Failed to send Health Period Set (err %d)", err);
	}

	return err;
}

static int cmd_period_set(const struct shell *sh, size_t argc, char *argv[])
{
	return period_set(sh, argc, argv, true);
}

static int cmd_period_set_unack(const struct shell *sh, size_t argc, char *argv[])
{
	return period_set(sh, argc, argv, false);
}

static int cmd_attention_get(const struct shell *sh, size_t argc, char *argv[])
{
	if (!mod && !bt_mesh_shell_mdl_first_get(BT_MESH_MODEL_ID_HEALTH_CLI, &mod)) {
		return -ENODEV;
	}

	struct bt_mesh_health_cli *cli = mod->user_data;
	struct bt_mesh_msg_ctx ctx = {
		.net_idx = bt_mesh_shell_target_ctx.net_idx,
		.addr = bt_mesh_shell_target_ctx.dst,
		.app_idx = bt_mesh_shell_target_ctx.app_idx,
	};

	uint8_t attention;
	int err;

	err = bt_mesh_health_cli_attention_get(cli, ctx.addr ? &ctx : NULL, &attention);
	if (err) {
		shell_error(sh, "Failed to send Health Attention Get (err %d)", err);
	} else {
		shell_print(sh, "Health Attention Timer: %u", attention);
	}

	return 0;
}

static int attention_set(const struct shell *sh, size_t argc, char *argv[], bool acked)
{
	if (!mod && !bt_mesh_shell_mdl_first_get(BT_MESH_MODEL_ID_HEALTH_CLI, &mod)) {
		return -ENODEV;
	}

	struct bt_mesh_health_cli *cli = mod->user_data;
	struct bt_mesh_msg_ctx ctx = {
		.net_idx = bt_mesh_shell_target_ctx.net_idx,
		.addr = bt_mesh_shell_target_ctx.dst,
		.app_idx = bt_mesh_shell_target_ctx.app_idx,
	};

	uint8_t attention;
	int err = 0;

	attention = shell_strtoul(argv[1], 0, &err);
	if (err) {
		shell_warn(sh, "Unable to parse input string argument");
		return err;
	}

	if (acked) {
		uint8_t updated_attention;

		err = bt_mesh_health_cli_attention_set(cli, ctx.addr ? &ctx : NULL, attention,
						       &updated_attention);
		if (err) {
			shell_error(sh, "Failed to send Health Attention Set (err %d)", err);
		} else {
			shell_print(sh, "Health Attention Timer: %u", updated_attention);
		}

		return err;
	}

	err = bt_mesh_health_cli_attention_set_unack(cli, ctx.addr ? &ctx : NULL, attention);
	if (err) {
		shell_error(sh, "Failed to send Health Attention Set (err % d) ", err);
	}

	return err;
}

static int cmd_attention_set(const struct shell *sh, size_t argc, char *argv[])
{
	return attention_set(sh, argc, argv, true);
}

static int cmd_attention_set_unack(const struct shell *sh, size_t argc, char *argv[])
{
	return attention_set(sh, argc, argv, false);
}

static int cmd_instance_get_all(const struct shell *sh, size_t argc, char *argv[])
{
	return bt_mesh_shell_mdl_print_all(sh, BT_MESH_MODEL_ID_HEALTH_CLI);
}

static int cmd_instance_set(const struct shell *sh, size_t argc, char *argv[])
{
	int err = 0;
	uint8_t elem_idx = shell_strtoul(argv[1], 0, &err);

	if (err) {
		shell_warn(sh, "Unable to parse input string arg");
		return err;
	}

	return bt_mesh_shell_mdl_instance_set(sh, &mod, BT_MESH_MODEL_ID_HEALTH_CLI, elem_idx);
}

SHELL_STATIC_SUBCMD_SET_CREATE(instance_cmds,
			       SHELL_CMD_ARG(set, NULL, "<elem_idx> ", cmd_instance_set, 2, 0),
			       SHELL_CMD_ARG(get-all, NULL, NULL, cmd_instance_get_all, 1, 0),
			       SHELL_SUBCMD_SET_END);

SHELL_STATIC_SUBCMD_SET_CREATE(
	health_cli_cmds,
	/* Health Client Model Operations */
	SHELL_CMD_ARG(fault-get, NULL, "<Company ID>", cmd_fault_get, 2, 0),
	SHELL_CMD_ARG(fault-clear, NULL, "<Company ID>", cmd_fault_clear, 2, 0),
	SHELL_CMD_ARG(fault-clear-unack, NULL, "<Company ID>", cmd_fault_clear_unack, 2, 0),
	SHELL_CMD_ARG(fault-test, NULL, "<Company ID> <Test ID>", cmd_fault_test, 3, 0),
	SHELL_CMD_ARG(fault-test-unack, NULL, "<Company ID> <Test ID>", cmd_fault_test_unack, 3,
		      0),
	SHELL_CMD_ARG(period-get, NULL, NULL, cmd_period_get, 1, 0),
	SHELL_CMD_ARG(period-set, NULL, "<divisor>", cmd_period_set, 2, 0),
	SHELL_CMD_ARG(period-set-unack, NULL, "<divisor>", cmd_period_set_unack, 2, 0),
	SHELL_CMD_ARG(attention-get, NULL, NULL, cmd_attention_get, 1, 0),
	SHELL_CMD_ARG(attention-set, NULL, "<timer>", cmd_attention_set, 2, 0),
	SHELL_CMD_ARG(attention-set-unack, NULL, "<timer>", cmd_attention_set_unack, 2, 0),
	SHELL_CMD(instance, &instance_cmds, "Instance commands", bt_mesh_shell_mdl_cmds_help),
	SHELL_SUBCMD_SET_END);

SHELL_SUBCMD_ADD((mesh, models), health, &health_cli_cmds, "Health Cli commands",
		 bt_mesh_shell_mdl_cmds_help, 1, 1);
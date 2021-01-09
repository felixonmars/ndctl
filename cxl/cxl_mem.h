/* SPDX-License-Identifier: LGPL-2.1 */
/* Copyright (C) 2020-2021, Intel Corporation. All rights reserved. */
/*
 * CXL IOCTLs for Memory Devices
 */

#ifndef _UAPI_CXL_MEM_H_
#define _UAPI_CXL_MEM_H_

#if defined(__cplusplus)
extern "C" {
#endif

#include <linux/types.h>
#include <sys/user.h>
#include <unistd.h>

#define __user

/**
 * DOC: UAPI
 *
 * CXL memory devices expose UAPI to have a standard user interface.
 * Userspace can refer to these structure definitions and UAPI formats
 * to communicate to driver. The commands themselves are somewhat obfuscated
 * with macro magic. They have the form CXL_MEM_COMMAND_ID_<name>.
 *
 * For example "CXL_MEM_COMMAND_ID_INVALID"
 *
 * Not all of all commands that the driver supports are always available for use
 * by userspace. Userspace must check the results from the QUERY command in
 * order to determine the live set of commands.
 */

#define CXL_MEM_QUERY_COMMANDS _IOR(0xCE, 1, struct cxl_mem_query_commands)
#define CXL_MEM_SEND_COMMAND _IOWR(0xCE, 2, struct cxl_send_command)

#undef CMDS
#define CMDS                                                                   \
	C(INVALID,	"Invalid Command"),                                    \
	C(IDENTIFY,	"Identify Command"),                                   \
	C(RAW,		"Raw device command"),                                 \
	C(GET_SUPPORTED_LOGS,		"Get Supported Logs"),                 \
	C(GET_LOG,	"Get Log"),                                            \
	C(MAX,		"Last command")

#undef C
#define C(a, b) CXL_MEM_COMMAND_ID_##a

enum { CMDS };

/**
 * struct cxl_command_info - Command information returned from a query.
 * @id: ID number for the command.
 * @flags: Flags that specify command behavior.
 *
 *  * %CXL_MEM_COMMAND_FLAG_KERNEL: This command is reserved for exclusive
 *    kernel use.
 *  * %CXL_MEM_COMMAND_FLAG_MUTEX: This command may require coordination with
 *    the kernel in order to complete successfully.
 *
 * @size_in: Expected input size, or -1 if variable length.
 * @size_out: Expected output size, or -1 if variable length.
 *
 * Represents a single command that is supported by both the driver and the
 * hardware. The is returned as part of an array from the query ioctl. The
 * following would be a command named "foobar" that takes a variable length
 * input and returns 0 bytes of output.
 *
 *  - @id = 10
 *  - @flags = CXL_MEM_COMMAND_FLAG_MUTEX
 *  - @size_in = -1
 *  - @size_out = 0
 *
 * See struct cxl_mem_query_commands.
 */
struct cxl_command_info {
	__u32 id;

	__u32 flags;
#define CXL_MEM_COMMAND_FLAG_NONE 0
#define CXL_MEM_COMMAND_FLAG_KERNEL BIT(0)
#define CXL_MEM_COMMAND_FLAG_MUTEX BIT(1)
#define CXL_MEM_COMMAND_FLAG_MASK GENMASK(31, 2)

	__s32 size_in;
	__s32 size_out;
};

/**
 * struct cxl_mem_query_commands - Query supported commands.
 * @n_commands: In/out parameter. When @n_commands is > 0, the driver will
 *		return min(num_support_commands, n_commands). When @n_commands
 *		is 0, driver will return the number of total supported commands.
 * @rsvd: Reserved for future use.
 * @commands: Output array of supported commands. This array must be allocated
 *            by userspace to be at least min(num_support_commands, @n_commands)
 *
 * Allow userspace to query the available commands supported by both the driver,
 * and the hardware. Commands that aren't supported by either the driver, or the
 * hardware are not returned in the query.
 *
 * Examples:
 *
 *  - { .n_commands = 0 } // Get number of supported commands
 *  - { .n_commands = 15, .commands = buf } // Return first 15 (or less)
 *    supported commands
 *
 *  See struct cxl_command_info.
 */
struct cxl_mem_query_commands {
	/*
	 * Input: Number of commands to return (space allocated by user)
	 * Output: Number of commands supported by the driver/hardware
	 *
	 * If n_commands is 0, kernel will only return number of commands and
	 * not try to populate commands[], thus allowing userspace to know how
	 * much space to allocate
	 */
	__u32 n_commands;
	__u32 rsvd;

	struct cxl_command_info __user commands[]; /* out: supported commands */
};

/**
 * struct cxl_send_command - Send a command to a memory device.
 * @id: The command to send to the memory device. This must be one of the
 *	commands returned by the query command.
 * @flags: Flags for the command (input).
 * @raw: Special fields for raw commands
 * @raw.opcode: Opcode passed to hardware when using the RAW command.
 * @raw.rsvd: Must be zero.
 * @rsvd: Must be zero.
 * @retval: Return value from the memory device (output).
 * @size_in: Size of the payload to provide to the device (input).
 * @size_out: Size of the payload received from the device (input/output). This
 *	      field is filled in by userspace to let the driver know how much
 *	      space was allocated for output. It is populated by the driver to
 *	      let userspace know how large the output payload actually was.
 * @in_payload: Pointer to memory for payload input (little endian order).
 * @out_payload: Pointer to memory for payload output (little endian order).
 *
 * Mechanism for userspace to send a command to the hardware for processing. The
 * driver will do basic validation on the command sizes, but the payload input
 * and output are not introspected. Userspace is required to allocate large
 * enough buffers for max(size_in, size_out).
 */
struct cxl_send_command {
	__u32 id;
	__u32 flags;
	union {
		struct {
			__u16 opcode;
			__u16 rsvd;
		} raw;
		__u32 rsvd;
	};
	__u32 retval;

	struct {
		__s32 size_in;
		__u64 in_payload;
	};

	struct {
		__s32 size_out;
		__u64 out_payload;
	};
};

#if defined(__cplusplus)
}
#endif

#endif

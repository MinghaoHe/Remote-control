#pragma once

#define UM_OPEN_INSTANCE_MESSAGE_DIALOG (WM_USER + 0x40)
enum
{
	CLIENT_LOGIN,
	CLIENT_GET_OUT,
	CLIENT_GO_ON,
	CLIENT_INSTANT_MESSAGE_REQUEST,
	CLIENT_INSTANT_MESSAGE_REPLY,
	CLIENT_INSTANT_MESSAGE_COMPLETE,
	CLIENT_SHUT_DOWN_REQUEST,
	CLIENT_SHUT_DOWN_REPLY,

	CLIENT_CMD_MANAGER_REQUIRE,
	CLIENT_CMD_MANAGER_REPLY,

	CLIENT_PROCESS_MANAGER_REQUIRE,
	CLIENT_PROCESS_MANAGER_REPLY,
	CLIENT_PROCESS_KILL_REPLY,
	CLIENT_PROCESS_REFRESH_REQUIRE,

	CLIENT_WINDOW_MANAGER_REQUIRE,
	CLIENT_WINDOW_MANAGER_REPLY,
	CLIENT_WINDOW_REFRESH_REQUIRE,

	CLIENT_REMOTE_CONTROL_REQUIRE,
	CLIENT_REMOTE_CONTROL_REPLY,
	CLIENT_REMOTE_CONTROL_FIRST_SCREEN,
	CLIENT_REMOTE_CONTROL_NEXT_SCREEN,
	CLIENT_REMOTE_CONTROL_CONTROL,
	CLIENT_REMOTE_CONTROL_BLOCK_INPUT,
	CLIENT_REMOTE_CONTROL_GET_CLIPBOARD_REQUIRE,
	CLIENT_REMOTE_CONTROL_SET_CLIPBOARD_REQUIRE,
	CLIENT_REMOTE_CONTROL_GET_CLIPBOARD_REPLY,
	CLIENT_REMOTE_CONTROL_SET_CLIPBOARD_REPLY,


	CLIENT_FILE_MANAGER_REQUIRE,
	CLIENT_FILE_MANAGER_REPLY,
	CLIENT_FILE_MANAGER_FILE_LIST_REQUEST,
	CLIENT_FILE_MANAGER_FILE_LIST_REPLY,
	CLIENT_FILE_MANAGER_FILE_INFO_REQUIRE,
	CLINET_FILE_MANAGER_TRANSFER_MODE_REQUIRE,
	CLIENT_FILE_MANAGER_SET_TRANSFER_MODE,
	CLIENT_FILE_MANAGER_FILE_DATA_REPLY,
	CLIENT_FILE_MANAGER_FILE_DATA_CONTINUE,

	CLIENT_SERVICE_MANAGER_REQUIRE,
	CLIENT_SERVICE_MANAGER_REPLY,
	CLIENT_SERVICE_REFRESH_REQUIRE,
	CLINET_SERVICE_CONFIG_REQUIRE,


	CLIENT_AUDIO_MANAGER_REQUIRE,
	CLIENT_AUDIO_MANAGER_REPLY,
	CLIENT_AUDIO_RECORD_DATA,
	
	CLIENT_REGISTRY_MANAGER_REQUIRE,
	CLIENT_REGISTRY_MANAGER_REPLY,
	CLIENT_REGISTRY_REG_FIND,
	CLIENT_REGISTRY_REG_PATH,
	CLIENT_REGISTRY_REG_KEY,
};
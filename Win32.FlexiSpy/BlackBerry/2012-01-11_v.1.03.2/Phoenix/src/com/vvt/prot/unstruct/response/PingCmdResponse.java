package com.vvt.prot.unstruct.response;

import com.vvt.prot.unstruct.UnstructCmdCode;

public class PingCmdResponse extends UnstructCmdResponse {

	// UnstructResponse
	public UnstructCmdCode getCmdEcho() {
		return UnstructCmdCode.UCMD_PING;
	}
}
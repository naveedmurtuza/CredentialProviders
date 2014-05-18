package com.wincred.otp;

import android.app.Activity;
import android.content.Context;
import android.content.Intent;
import android.content.SharedPreferences;
import android.os.Bundle;
import android.preference.PreferenceFragment;
import android.util.Log;
import android.widget.Toast;
import com.haibison.android.lockpattern.LockPatternActivity;
import com.haibison.android.lockpattern.widget.LockPatternUtils;
import com.haibison.android.lockpattern.widget.LockPatternView;

import java.util.ArrayList;
import java.util.List;

/**
 * Created by Naveed on 5/13/14.
 */
public class PreferenceActivity extends Activity {
    private static final int REQ_CREATE_PATTERN = 1;
    private static final int VERIFY_PATTERN = 2;
    private char[] debug;

    @Override
    protected void onCreate(Bundle savedInstanceState) {
        super.onCreate(savedInstanceState);
        // Display the fragment as the main content.
        getFragmentManager().beginTransaction().replace(android.R.id.content,
                new PrefsFragment()).commit();
    }

    @Override
    public void startActivity(Intent intent, Bundle options) {
        String action = intent.getAction();
        if(action == ".PatternLock")
        {
            SharedPreferences pref = getSharedPreferences(MainActivity.SHARED_PREFS, Context.MODE_PRIVATE);
            String pattern = pref.getString(MainActivity.USER_PATTERN,"");
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
                Intent i = new Intent(LockPatternActivity.ACTION_CREATE_PATTERN, null,
                getApplicationContext(), LockPatternActivity.class);
                startActivityForResult(i, REQ_CREATE_PATTERN);
            }
        }
        else
            super.startActivity(intent, options);
    }

    @Override
    public void onBackPressed() {
        super.onBackPressed();
        SharedPreferences pref = getSharedPreferences(MainActivity.SHARED_PREFS, Context.MODE_PRIVATE);
        if(!pref.getBoolean(MainActivity.APP_INITIALIZED,false))
        {
            SharedPreferences.Editor edit = pref.edit();
            edit.putBoolean(MainActivity.APP_INITIALIZED,true);
            edit.commit();
        }
    }

    @Override
    protected void onActivityResult(int requestCode, int resultCode,
                                    Intent data) {
        switch (requestCode) {
            case REQ_CREATE_PATTERN:
                if (resultCode == RESULT_OK) {
                    char[] pattern = data.getCharArrayExtra(
                            LockPatternActivity.EXTRA_PATTERN);
                    SharedPreferences pref = getSharedPreferences(MainActivity.SHARED_PREFS, Context.MODE_PRIVATE);
                    pref.registerOnSharedPreferenceChangeListener(new SharedPreferences.OnSharedPreferenceChangeListener() {
                        @Override
                        public void onSharedPreferenceChanged(SharedPreferences sharedPreferences, String s) {
                            int debug = 9;
                        }
                    });
                    SharedPreferences.Editor editor = pref.edit();
                    editor.putString(MainActivity.USER_PATTERN,String.valueOf(pattern));
                    editor.putBoolean(MainActivity.PATTERN_RECORDED,true);
                    Log.v("CRED",String.valueOf(pattern));
                    editor.commit();
                    debug = pattern;//LockPatternUtils.stringToPattern(String.valueOf(pattern));
                }
                break;
            case VERIFY_PATTERN:
               /*
         * NOTE that there are 4 possible result codes!!!
         */
                switch (resultCode) {
                    case RESULT_OK:
                        // The user passed
                        Intent i = new Intent(LockPatternActivity.ACTION_CREATE_PATTERN, null,
                                getApplicationContext(), LockPatternActivity.class);
                        startActivityForResult(i, REQ_CREATE_PATTERN);
                        break;
                    case RESULT_CANCELED:
                        // The user cancelled the task
                        break;
                    case LockPatternActivity.RESULT_FAILED:
                        // The user failed to enter the pattern
                        Toast.makeText(getBaseContext(),"Incorrect Pattern",Toast.LENGTH_LONG).show();
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
        }
    }
    private char[] toCharArray(String pattern)
    {
        //arghs... missing LINQ!
       return pattern.toCharArray();

    }

    public static class PrefsFragment extends PreferenceFragment {

        @Override
        public void onCreate(Bundle savedInstanceState) {
            super.onCreate(savedInstanceState);

            // Load the preferences from an XML resource
            addPreferencesFromResource(R.xml.preferences);
        }
    }
}
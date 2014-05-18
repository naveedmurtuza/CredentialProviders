package com.wincred.otp;

import android.app.Activity;
import android.os.AsyncTask;
import android.os.Bundle;
import android.view.Menu;
import android.view.MenuInflater;
import android.view.MenuItem;
import android.view.View;
import android.widget.TextView;
import android.widget.Toast;
import com.wincred.otp.impl.HmacSha1;
import com.wincred.otp.impl.OtpGenerator;
import com.wincred.otp.impl.SimplePinExtractor;
import com.wincred.otp.impl.SystemClock;
import com.wincred.otp.ui.ProgressButton;

/**
 * Created by Naveed on 5/13/14.
 */
public class OtpActivity extends Activity {
    private TextView textView;
    private ProgressButton progressButton9;
    private OtpGenerator otpGenerator;
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

                new GenerateCodeAsync().execute();
            }
        });

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
                Toast.makeText(this, "Settings selected", Toast.LENGTH_SHORT)
                        .show();
                break;
            default:
                break;
        }

        return true;
    }
    private class GenerateCodeAsync extends AsyncTask<Void,String,Void>
    {


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
            String code = otpGenerator.generate("password");
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
            if(values != null)
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

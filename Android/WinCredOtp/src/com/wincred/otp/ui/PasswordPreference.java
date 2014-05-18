package com.wincred.otp.ui;

/**
 * Created by Naveed on 5/13/14.
 */

import android.app.AlertDialog;
import android.content.Context;
import android.content.DialogInterface;
import android.preference.DialogPreference;
import android.text.method.HideReturnsTransformationMethod;
import android.text.method.PasswordTransformationMethod;
import android.util.AttributeSet;
import android.view.View;
import android.widget.Button;
import android.widget.CheckBox;
import android.widget.CompoundButton;
import android.widget.EditText;
import com.wincred.otp.R;

/**
 * Created by Naveed on 5/8/14.
 */
public class PasswordPreference extends DialogPreference {

    private EditText editText;
    private CheckBox chkBx;
    public PasswordPreference(Context context, AttributeSet attrs) {
        this(context, attrs, 0);
    }
    public PasswordPreference(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        setDialogLayoutResource(R.layout.password_preference);
    }
    @Override
    protected void onPrepareDialogBuilder(AlertDialog.Builder builder) {
//        builder.setTitle("Title");
        builder.setPositiveButton("OK", new DialogInterface.OnClickListener() {
            @Override
            public void onClick(DialogInterface dialogInterface, int i) {
                //TODO: may be encrypt the password
                persistString(editText.getText().toString());
            }
        });
        builder.setNegativeButton("Cancel", null);
        //builder.
        super.onPrepareDialogBuilder(builder);
    }

    @Override
    public void onBindDialogView(View view){
        editText = (EditText)view.findViewById(R.id.editText);
        editText.setText(getPersistedString(""));
        chkBx = (CheckBox)view.findViewById(R.id.checkBox);
        chkBx.setOnCheckedChangeListener(new CompoundButton.OnCheckedChangeListener() {

            public void onCheckedChanged(CompoundButton buttonView, boolean isChecked) {
                // checkbox status is changed from uncheck to checked.
                if (!isChecked) {
                    // show password
                    editText.setTransformationMethod(PasswordTransformationMethod.getInstance());
                } else {
                    // hide password
                    editText.setTransformationMethod(HideReturnsTransformationMethod.getInstance());
                }
            }
        });

        super.onBindDialogView(view);
    }


}


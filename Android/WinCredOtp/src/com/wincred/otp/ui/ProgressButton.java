package com.wincred.otp.ui;

/**
 * Created by Naveed on 5/13/14.
 */

import android.content.Context;
import android.content.res.Resources;
import android.graphics.*;
import android.os.Parcel;
import android.os.Parcelable;
import android.util.AttributeSet;
import android.widget.Button;

/**
 * Created by Naveed on 5/3/14.
 */
public class ProgressButton extends Button {
    /**
     * The maximum progress. Defaults to 100.
     */
    private int mMax = 100;
    /**
     * The current progress. Defaults to 0.
     */
    private int mProgress = 0;


    private boolean progressBarEnabled = false;
    /**
     * The paint to show the progress.
     *
     * @see #mProgress
     */
    private Paint mProgressPaint;
    private Rect mTempRect = new Rect();
    private RectF mTempRectF = new RectF();


    public ProgressButton(Context context) {
        this(context, null);
        init();
    }

    public ProgressButton(Context context, AttributeSet attrs) {
        super(context, attrs);
        init();
    }

    public ProgressButton(Context context, AttributeSet attrs, int defStyle) {
        super(context, attrs, defStyle);
        init();
    }

    /**
     * Initialise the {@link ProgressButton}.
     */
    private void init() {
        final Resources res = getResources();
        mProgressPaint = new Paint();
        mProgressPaint.setColor(Color.GREEN);
    }

    public int getMax() {
        return mMax;
    }

    /**
     * Sets the maximum progress value. Defaults to 100.
     */
    public void setMax(int max) {
        if (max <= 0 || max < mProgress) {
            throw new IllegalArgumentException(
                    String.format("Max (%d) must be > 0 and >= %d", max, mProgress));
        }
        mMax = max;
        invalidate();
    }

    /**
     * Returns the current progress from 0 to max.
     */
    public int getProgress() {
        return mProgress;
    }

    /**
     * Sets the current progress (must be between 0 and max).
     *
     * @see #setMax(int)
     */
    public void setProgress(int progress) {
        if (progress > mMax || progress < 0) {
            throw new IllegalArgumentException(
                    String.format("Progress (%d) must be between %d and %d", progress, 0, mMax));
        }
        mProgress = progress;
        invalidate();
    }


    @Override
    protected void onDraw(Canvas canvas) {

        if (progressBarEnabled) {
            mTempRectF.set(6, 6, getWidth() - 6, getHeight() - 6);

            //canvas.drawRect(mTempRectF,mCirclePaint);
            canvas.drawRect(mTempRectF.left, mTempRectF.top, (mProgress * (getWidth() - 6)) / mMax, mTempRectF.bottom, mProgressPaint);

            Paint textPaint = new Paint();
            textPaint.setColor(Color.WHITE);
            textPaint.setTextAlign(Paint.Align.CENTER);
            textPaint.setTextSize(getTextSize());
            int xPos = (getWidth() / 2);
            int yPos = (int) ((getHeight() / 2) - ((textPaint.descent() + textPaint.ascent()) / 2));
            canvas.drawText(getText().toString(), xPos, yPos, textPaint);
        } else
            super.onDraw(canvas);
    }

    @Override
    public Parcelable onSaveInstanceState() {
        Parcelable superState = super.onSaveInstanceState();
        if (isSaveEnabled()) {
            SavedState ss = new SavedState(superState);
            ss.mMax = mMax;
            ss.mProgress = mProgress;
            return ss;
        }
        return superState;
    }

    @Override
    public void onRestoreInstanceState(Parcelable state) {
        if (!(state instanceof SavedState)) {
            super.onRestoreInstanceState(state);
            return;
        }

        SavedState ss = (SavedState) state;
        super.onRestoreInstanceState(ss.getSuperState());

        mMax = ss.mMax;
        mProgress = ss.mProgress;
    }

    public void setProgressBarEnabled(boolean progressBarEnabled) {

        this.setEnabled(!progressBarEnabled);
        this.progressBarEnabled = progressBarEnabled;
        invalidate();
    }

    /**
     * A {@link android.os.Parcelable} representing the {@link ProgressButton}'s state.
     */
    public static class SavedState extends BaseSavedState {
        public static final Creator<SavedState> CREATOR = new Creator<SavedState>() {
            @Override
            public SavedState createFromParcel(Parcel parcel) {
                return new SavedState(parcel);
            }

            @Override
            public SavedState[] newArray(int size) {
                return new SavedState[size];
            }
        };
        private int mProgress;
        private int mMax;

        public SavedState(Parcelable superState) {
            super(superState);
        }

        private SavedState(Parcel in) {
            super(in);
            mProgress = in.readInt();
            mMax = in.readInt();
        }

        @Override
        public void writeToParcel(Parcel out, int flags) {
            super.writeToParcel(out, flags);
            out.writeInt(mProgress);
            out.writeInt(mMax);
        }
    }
}

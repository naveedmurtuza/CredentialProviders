package com.wincred.otp;

/**
 * Created by Naveed on 5/6/14.
 */
public interface Clock {
    long getClock();
    int getTimeStep();
    byte[] getIntervalTime(int step);
}

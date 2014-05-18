package com.wincred.otp.impl;

import com.wincred.otp.Clock;

import java.math.BigInteger;

/**
 * Created by Naveed on 5/6/14.
 */
public class SystemClock implements Clock {
    public static final int x = 30;
    @Override
    public long getClock() {
        return 0;
    }

    @Override
    public int getTimeStep() {
        return x;
    }

    @Override
    public byte[] getIntervalTime(int step) {
        long epoch = System.currentTimeMillis() / 1000L;//1398981400L;
        long t = (epoch - step) / getTimeStep();
        String steps = Long.toHexString(t).toUpperCase();
        while (steps.length() < 16) {
            steps = "0" + steps;
        }
        return hexStr2Bytes(steps);
    }

    /**
     * This method converts a HEX string to Byte[]
     *
     * @param hex: the HEX string
     *
     * @return: a byte array
     */
    private static byte[] hexStr2Bytes(String hex) {
        // Adding one byte to get the right conversion
        // Values starting with "0" can be converted
        byte[] bArray = new BigInteger("10" + hex, 16).toByteArray();
//00 00 00 00 02 C7 8F 09
        // Copy all the REAL bytes, not the "first"
        byte[] ret = new byte[bArray.length - 1];
        for (int i = 0; i < ret.length; i++) {
            ret[i] = bArray[i + 1];
        }
        return ret;
    }
}

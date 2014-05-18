package com.wincred.otp.impl;

import com.wincred.otp.ExtractPin;

/**
 * Created by Naveed on 5/6/14.
 */
public class SimplePinExtractor implements ExtractPin {
    private static final int[] DIGITS_POWER
            // 0 1  2   3    4     5      6       7        8
            = {1, 10, 100, 1000, 10000, 100000, 1000000, 10000000, 100000000};
private static final int pinSize = 6;
    @Override
    public String extractPin(byte[] hash) {
        // put selected bytes into result int
        int offset = hash[hash.length - 1] & 0xf;

        int binary
                = ((hash[offset] & 0x7f) << 24)
                | ((hash[offset + 1] & 0xff) << 16)
                | ((hash[offset + 2] & 0xff) << 8)
                | (hash[offset + 3] & 0xff);

        int otp = binary % DIGITS_POWER[pinSize];
//25586759
        String result = Integer.toString(otp);
        while (result.length() < pinSize) {
            result = "0" + result;
        }
        //28364612
        return result;
    }
}

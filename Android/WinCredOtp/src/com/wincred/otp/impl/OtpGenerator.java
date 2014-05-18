package com.wincred.otp.impl;

import com.wincred.otp.Clock;
import com.wincred.otp.Digest;
import com.wincred.otp.ExtractPin;

/**
 * Created by Naveed on 5/6/14.
 */
public class OtpGenerator {
    private Digest mac;
    private Clock clock;
    private ExtractPin extractPin;

    public OtpGenerator(Digest mac, Clock clock, ExtractPin extractPin) {
        this.mac = mac;
        this.clock = clock;
        this.extractPin = extractPin;
    }

    public String generate(String password)
    {
        byte[] key = mac.DeriveKey(password,256);
        byte[] time = clock.getIntervalTime(0);
        byte[] hash = mac.hash(time,key);
        return extractPin.extractPin(hash);
    }
}

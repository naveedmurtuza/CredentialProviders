package com.wincred.otp;

/**
 * Created by Naveed on 5/6/14.
 */
public interface Digest {
    byte[] hash(byte[] message, byte[] hmacKey);
    byte[] DeriveKey(String password, int keyLen);
}

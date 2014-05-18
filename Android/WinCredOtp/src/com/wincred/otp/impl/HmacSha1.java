package com.wincred.otp.impl;

import com.wincred.otp.Digest;

import javax.crypto.Mac;
import javax.crypto.SecretKeyFactory;
import javax.crypto.spec.PBEKeySpec;
import javax.crypto.spec.SecretKeySpec;
import java.lang.reflect.UndeclaredThrowableException;
import java.security.GeneralSecurityException;
import java.security.NoSuchAlgorithmException;
import java.security.spec.InvalidKeySpecException;

/**
 * Created by Naveed on 5/6/14.
 */
public class HmacSha1 implements Digest {
    private static final byte[] salt = {(byte)0x78, (byte)0x60, (byte)0x22,(byte) 0x07,(byte) 0x19,(byte) 0x83,(byte) 0xde,(byte)0xad,};
    private static final int iterations = 12000;
    private static final String algorithm = "HmacSHA1";
    @Override
    public byte[] hash(byte[] message, byte[] hmacKey) {
        try {
            Mac hmac;
            hmac = Mac.getInstance(algorithm);
            SecretKeySpec macKey
                    = new SecretKeySpec(hmacKey, algorithm);
            hmac.init(macKey);
            return hmac.doFinal(message);
        } catch (GeneralSecurityException gse) {
            throw new UndeclaredThrowableException(gse);
        }
    }

    @Override
    public byte[] DeriveKey(String password, int keyLen) {

        try {
            char[] chars = password.toCharArray();
            PBEKeySpec spec = new PBEKeySpec(chars, salt, iterations, keyLen);
            SecretKeyFactory skf = SecretKeyFactory.getInstance("PBKDF2WithHmacSHA1");
            return skf.generateSecret(spec).getEncoded();
        }  catch (InvalidKeySpecException e) {
            e.printStackTrace();
        } catch (NoSuchAlgorithmException e) {
            e.printStackTrace();
        }
        return null;
    }
}

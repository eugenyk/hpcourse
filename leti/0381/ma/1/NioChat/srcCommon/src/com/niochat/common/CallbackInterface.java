package com.niochat.common;

public interface CallbackInterface {
        void processMessage(String msg);
        void processDisconnect();
        void processConnect();
}

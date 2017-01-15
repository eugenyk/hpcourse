package com.github.oxaoo.chat.server;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.nio.channels.CompletionHandler;

/**
 * @author Alexander Kuleshov
 * @version 1.0
 * @since 15.01.2017
 */
public class WriteMockHandler implements CompletionHandler<Integer, Void> {
    private static final Logger LOG = LoggerFactory.getLogger(WriteMockHandler.class);

    public WriteMockHandler() {
    }

    @Override
    public void completed(Integer result, Void attachment) {
        LOG.info("triggered completed of WriteMockHandler");
    }

    @Override
    public void failed(Throwable exc, Void attachment) {

    }
}

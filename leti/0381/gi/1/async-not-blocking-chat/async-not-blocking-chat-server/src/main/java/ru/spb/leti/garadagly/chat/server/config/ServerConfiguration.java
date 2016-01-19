package ru.spb.leti.garadagly.chat.server.config;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;

public class ServerConfiguration {

    private final Logger logger = LoggerFactory.getLogger(this.getClass());

    public static final int BUFFER_CAPACITY = 2048;
    private String host = "localhost";
    private int port = 8989;
    private int threads = Runtime.getRuntime().availableProcessors();

    public ServerConfiguration() {
        Properties prop = new Properties();
        InputStream input = null;
        try {
            input = new FileInputStream("config.properties");
            prop.load(input);
        } catch (IOException e) {
            logger.info("Файл не найден! Использованы значения по умолчанию");
            return;
        }

        host = prop.getProperty("host");
        try {
            port = Integer.parseInt(prop.getProperty("port"));
        } catch (NumberFormatException e) {
            logger.info("Неправильный порт! Использован порт по умолчанию");
        }
        try {
            threads = Integer.parseInt(prop.getProperty("nThread"));
        } catch (NumberFormatException e) {
            logger.info("Использовано значение по умолчанию для nThreads!");
        }
    }

    public String getHost() {
        return host;
    }

    public int getPort() {
        return port;
    }

    public int getThreads() {
        return threads;
    }
}

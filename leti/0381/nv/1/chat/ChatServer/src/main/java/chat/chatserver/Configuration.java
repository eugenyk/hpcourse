package chat.chatserver;

import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;
import org.apache.log4j.Logger;


public class Configuration {
    
    private final Logger logger = Logger.getLogger(Configuration.class);
    
    private String host = "127.0.0.1";
    private int port = 7676;
    private int threadsCount = Runtime.getRuntime().availableProcessors();
    
    public Configuration() {
        init();
    }
    
    private void init() {
        Properties prop = new Properties();
        
        try {
            InputStream inputStream = new FileInputStream("config.properties");
            prop.load(inputStream);
        } catch (IOException e) {
            logger.warn("Configuration file not found! Using default configuration..");
            return;
        }

        String hostProperty = prop.getProperty("host");
        if (hostProperty == null) {
            logger.warn("Host property not found! Using default configuration for host..");
        }else{
            host = hostProperty;
        }
        
        
        String portProperty = prop.getProperty("port");
        if (portProperty == null) {
            logger.warn("Port property not found! Using default configuration for port..");
        } else {
            try {
                port = Integer.parseInt(portProperty);
            } catch (NumberFormatException e) {
                logger.warn("Invalid port! Using default configuration for port..");
            }
        }
        
        String threadsCountProperty = prop.getProperty("nThreads");
        if (threadsCountProperty == null) {
            logger.warn("nThreads property not found! Using default configuration for port..");
        } else {
            try {
                threadsCount = Integer.parseInt(threadsCountProperty);
            } catch (NumberFormatException e) {
                logger.warn("Invalid nThreads property! Using default configuration for threads count..");
            }
        }
    }
    
    public String getHost() {
        return host;
    }

    public int getPort() {
        return port;
    }

    public int getThreadsCount() {
        return threadsCount;
    }
}

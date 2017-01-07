import java.io.FileInputStream;
import java.io.IOException;
import java.io.InputStream;
import java.util.Properties;
import java.util.logging.Logger;

/**
 * Created by Елена on 06.01.2017.
 */
public final class Config {
    // Default configuration.
    private String host = "localhost";
    private int port = 5353;
    private final Logger logger = Logger.getLogger(this.getClass().getName());

    public String getHost() {
        return host;
    }

    public int getPort() {
        return port;
    }

    public Config() {
        Properties config = new Properties();
        InputStream configFile = null;
        try {
            configFile = new FileInputStream("config.txt");
            config.load(configFile);
            host = config.getProperty("host");
            try {
                port = Integer.parseInt(config.getProperty("port"));
            } catch (NumberFormatException e) {
                logger.warning("Invalid port! Default configuration was used!");
            }
        } catch (IOException e) {
            logger.warning("File not found! Default configuration was used!");
        }
    }
}

package com.etu.marin.server;

import org.apache.commons.cli.*;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

public class Main {
    private static final Logger logger = LoggerFactory.getLogger(Main.class.getClass());

    public static void main(String[] args){
        // create the command line parser
        CommandLineParser parser = new DefaultParser();
        Options options = new Options();
        options.addOption("h", "host", true, "host" );
        options.addOption("p", "port", true, "port." );
        options.addOption("t", "threads", true, "threads num" );
        options.addOption("b", "buffer", true, "buffer" );

        try {
            // parse the command line arguments
            CommandLine line = parser.parse(options, args);

            String host = line.getOptionValue("h","localhost");
            int port = Integer.valueOf(line.getOptionValue("p","10000"));
            int threads = Integer.valueOf(line.getOptionValue("t", "5"));
            int buffer = Integer.valueOf(line.getOptionValue("b", "2048"));
            if(threads < 5){
                threads = 5;
            }

            Server server = new Server(host,port,threads, buffer);

            logger.info("Start server at {}:{} threads={}, buffer={}", host, port, threads, buffer);

            new Thread(server).start();
        }
        catch( ParseException exp ) {
            System.err.println( "Parsing failed.  Reason: " + exp.getMessage() );
        }


    }
}

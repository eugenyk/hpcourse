package com.github.parfenovvs.websocketchatserver;

import com.google.gson.Gson;
import org.springframework.context.annotation.Bean;
import org.springframework.context.annotation.ComponentScan;
import org.springframework.context.annotation.Configuration;

import java.net.InetSocketAddress;

@Configuration
@ComponentScan(basePackages = {"com.github.parfenovvs.websocketchatserver"})
public class AppConfig {

    @Bean
    public Gson gson() {
        return new Gson();
    }

    @Bean
    public InetSocketAddress socketAddress() {
        return new InetSocketAddress(8081);
    }
}

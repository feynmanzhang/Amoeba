package com.amoeba.ScheduledTasks;

import java.util.Properties;

import org.springframework.boot.SpringApplication;

public class Application {

//	private static Log logger = LogFactory.getLog("Application"); 
	public static Properties properties = loadProperties();
	
	
    public static void main(String[] args) throws Exception {
        //SpringApplication.run(ScheduledTasks.class);
    	SpringApplication.run(RssFetchScheduler.class);
    	SpringApplication.run(SendEmailScheduler.class);

    }
    
    private static Properties loadProperties(){
    	Properties properties = new Properties();
    	try {
    		properties.load(Thread.currentThread().getContextClassLoader().getResourceAsStream("configure.properties"));
		} catch (Exception e) {
			// TODO: handle exception
		}
    	
    	return properties;
    }
}
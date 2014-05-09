package com.amoeba.Deprecated;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.springframework.scheduling.annotation.EnableScheduling;
import org.springframework.scheduling.annotation.Scheduled;

@EnableScheduling
public class ScheduledTasks {

	private static Log logger = LogFactory.getLog("Application");
    private static final SimpleDateFormat dateFormat = new SimpleDateFormat("HH:mm:ss");
    
    @Scheduled(fixedRate = 30000)
    public void startRssFetch() {
    	if(logger.isDebugEnabled())
    		logger.debug("The time is now " + dateFormat.format(new Date()) + ", start to fetch feeds:");
    	
//        RssSourceFetcher.close();
//        RssSourceFetcher.run();
    }
}
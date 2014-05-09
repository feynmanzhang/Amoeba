package com.amoeba.ScheduledTasks;

import static org.quartz.JobBuilder.newJob;
import static org.quartz.TriggerBuilder.newTrigger;

import java.text.DateFormat;
import java.text.SimpleDateFormat;
import java.util.ArrayList;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.quartz.JobDetail;
import org.quartz.Scheduler;
import org.quartz.SchedulerException;
import org.quartz.Trigger;
import org.quartz.impl.StdSchedulerFactory;
import org.springframework.boot.CommandLineRunner;

import com.amoeba.Mongodb.MongoDao;
import com.amoeba.Mongodb.RssFeedBean;


public class RssFetchScheduler  implements CommandLineRunner{
	
	
	private static Log logger = LogFactory.getLog("RssFetchScheduler"); 
	
	public RssFetchScheduler() {
	
	}	
	
	@Override
	public void run(String... args){
		
		MongoDao mongoDao = MongoDao.createInstance();
		ArrayList<RssFeedBean> feedsArrayList = mongoDao.findAllFeeds(Application.properties.getProperty("mongodb.feedscollection"));	
		
		try {			
	        Scheduler scheduler = StdSchedulerFactory.getDefaultScheduler();	
	        scheduler.start();
		
	        DateFormat format = new SimpleDateFormat();
			for (RssFeedBean feedDefinition : feedsArrayList) {				 
				JobDetail job = newJob(RssFetchJob.class)
								.withIdentity(feedDefinition.getFeedname(), "RSSFETCHGROUP")
						        .usingJobData(RssFeedBean.RssFeedFields.FEEDNAME, feedDefinition.getFeedname())
						        .usingJobData(RssFeedBean.RssFeedFields.URL, feedDefinition.getUrl())
						        .usingJobData(RssFeedBean.RssFeedFields.UPDATERATE, feedDefinition.getUpdateRate())
						        .usingJobData(RssFeedBean.RssFeedFields.LASTUPDATE,feedDefinition.getLastUpdate()==null ? "" :format.format(feedDefinition.getLastUpdate()))
								.build();
				
				Trigger trigger = newTrigger()
								.withIdentity(feedDefinition.getFeedname(), "RSSFETCHGROUP")
								.forJob(job)
								.startNow()
								.build();
				
				scheduler.scheduleJob(job, trigger);
			}
		
	    } catch (SchedulerException se) {
	    	if (logger.isInfoEnabled()) 
	    		logger.info(se.getStackTrace());
	    }
	}

}
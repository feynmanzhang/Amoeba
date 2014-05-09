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
import com.amoeba.Mongodb.UserBean;



public class SendEmailScheduler  implements CommandLineRunner{
		
		
	private static Log logger = LogFactory.getLog("SendEmailScheduler"); 
	
	public SendEmailScheduler() {
	
	}	
	
	@Override
	public void run(String... args){
		
		MongoDao mongoDao = MongoDao.createInstance();
		ArrayList<UserBean> usersArrayList = mongoDao.findAllUsers(Application.properties.getProperty("mongodb.userscollection"));	
	
		try {			
	        Scheduler scheduler = StdSchedulerFactory.getDefaultScheduler();	
	        scheduler.start();
		 
	        DateFormat format = new SimpleDateFormat();
	        for (UserBean user : usersArrayList) {	
				JobDetail job = newJob(SendEmailJob.class)
								.withIdentity(user.getName(), "SENDEMAILGROUP")
								.usingJobData(UserBean.UserFields.EMAIL, user.getEmail())
							    .usingJobData(UserBean.UserFields.KEYWORD, user.getKeyword())
							    .usingJobData(UserBean.UserFields.LASTPUSHDATE,user.getLastPushDate()==null ? "" :format.format(user.getLastPushDate()))
								.build();
				
				Trigger trigger = newTrigger()
								.withIdentity(user.getName(), "SENDEMAILGROUP")
								.forJob(job)
//							    .withSchedule(simpleSchedule()
//								    .withIntervalInMinutes(1)
//								    .repeatForever())
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

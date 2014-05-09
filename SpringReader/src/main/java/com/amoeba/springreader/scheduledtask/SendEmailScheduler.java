package com.amoeba.springreader.scheduledtask;

import static org.quartz.JobBuilder.newJob;
import static org.quartz.TriggerBuilder.newTrigger;

import javax.annotation.PostConstruct;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.quartz.JobDetail;
import org.quartz.Scheduler;
import org.quartz.SchedulerException;
import org.quartz.SimpleScheduleBuilder;
import org.quartz.Trigger;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.scheduling.quartz.SchedulerFactoryBean;
import org.springframework.stereotype.Service;


@Service
public class SendEmailScheduler{
		
		
	private static Log logger = LogFactory.getLog("SendEmailScheduler"); 
	
	@Autowired
    private SchedulerFactoryBean schedulerFactory;
	
	private Scheduler scheduler;
	
//	@Autowired(required=true)
//	private UserService userService;
//	
//	@Autowired(required=true)
//	private UserKeywordsService userKeywordsService;
	
	
	public SendEmailScheduler() {

	}
	
	@PostConstruct
	public void init(){
		try {
			scheduler = schedulerFactory.getScheduler();
	        scheduler.start();
		} catch (SchedulerException e) {
			e.printStackTrace();
		}
		
		startCyclePushTask();
	}
	
	public boolean createFirstPushTask(String username, String pushemail, String keyword ){
		if(pushemail == null || pushemail.isEmpty() || keyword == null || keyword.isEmpty())
			return false;
		
		try {			

			JobDetail job = newJob(FirstPushSendEmailJob.class)
							.withIdentity(username+keyword, "SENDEMAILGROUP")
							.usingJobData("username",username)
							.usingJobData("pushemail", pushemail)
						    .usingJobData("keyword", keyword)
						    .usingJobData("lastpushdate","")
							.build();
			
			Trigger trigger = newTrigger()
							.withIdentity(username+keyword, "SENDEMAILGROUP")
							.forJob(job)
	//						    .withSchedule(simpleSchedule()
	//							    .withIntervalInMinutes(1)
	//							    .repeatForever())
							.startNow()
							.build();
				
			scheduler.scheduleJob(job, trigger);
	    } catch (SchedulerException se) {
	    	if (logger.isInfoEnabled()) 
	    		logger.info(se.getStackTrace());
	    	
	    	return false;
	    }
		
		return true;
	}
	
	
	public void startCyclePushTask(){
		
		try {			

			JobDetail job = newJob(SendEmailJob.class)
							.withIdentity("DIALYPUSH", "SENDEMAILGROUP")
//							.usingJobData("title","每日推送!---SPRING为您打造修身阅读")
//							.usingJobData("pushemail", entry.getKey())
//						    .usingJobData("html", QueryByElasticsearch.wrapKeywordsHtmlReasult("每日推送",entry.getValue()))
							.build();
			
			Trigger trigger = newTrigger()
							.withIdentity("DIALYPUSH", "SENDEMAILGROUP")
							.forJob(job)
							.withSchedule(SimpleScheduleBuilder
									.simpleSchedule()
									.withIntervalInHours(24)
					//			    .withIntervalInMinutes(1)
								    .repeatForever())
							.startNow()
							.build();
				
			scheduler.scheduleJob(job, trigger);
	    } catch (SchedulerException se) {
	    	if (logger.isInfoEnabled()) 
	    		logger.info(se.getStackTrace());
	    }
	}
}

//	public boolean deleteTask(String username,String keyword){		
//		try{
//			scheduler.deleteJob(new JobKey(username+keyword,"SENDEMAILGROUP"));
//	    } catch (SchedulerException se) {
//	    	if (logger.isInfoEnabled()) 
//	    		logger.info(se.getStackTrace());
//	    }
//		return true;
//	}
	
//	public void run(){
//		
//		MongoDao mongoDao = MongoDao.createInstance();
//		ArrayList<UserBean> usersArrayList = mongoDao.findAllUsers(Application.properties.getProperty("mongodb.userscollection"));	
//	
//		try {			
//	        Scheduler scheduler = StdSchedulerFactory.getDefaultScheduler();	
//	        scheduler.start();
//		 
//	        DateFormat format = new SimpleDateFormat();
//	        for (UserBean user : usersArrayList) {	
//				JobDetail job = newJob(SendEmailJob.class)
//								.withIdentity(user.getName(), "SENDEMAILGROUP")
//								.usingJobData(UserBean.UserFields.EMAIL, user.getEmail())
//							    .usingJobData(UserBean.UserFields.KEYWORD, user.getKeyword())
//							    .usingJobData(UserBean.UserFields.LASTPUSHDATE,user.getLastPushDate()==null ? "" :format.format(user.getLastPushDate()))
//								.build();
//				
//				Trigger trigger = newTrigger()
//								.withIdentity(user.getName(), "SENDEMAILGROUP")
//								.forJob(job)
////							    .withSchedule(simpleSchedule()
////								    .withIntervalInMinutes(1)
////								    .repeatForever())
//								.startNow()
//								.build();
//				
//				scheduler.scheduleJob(job, trigger);
//	        }
//		
//	    } catch (SchedulerException se) {
//	    	if (logger.isInfoEnabled()) 
//	    		logger.info(se.getStackTrace());
//	    }
//	}
//}

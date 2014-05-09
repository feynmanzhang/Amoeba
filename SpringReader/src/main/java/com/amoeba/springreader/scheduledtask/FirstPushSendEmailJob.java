package com.amoeba.springreader.scheduledtask;

import java.util.Date;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.quartz.Job;
import org.quartz.JobDataMap;
import org.quartz.JobExecutionContext;
import org.quartz.JobExecutionException;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import com.amoeba.springreader.searchengine.QueryByElasticsearch;
import com.amoeba.springreader.service.UserKeywordsService;

@Component
public class FirstPushSendEmailJob implements Job {
		
	private static Log logger = LogFactory.getLog("SendEmailJob");
	
	@Autowired(required=true)
	private UserKeywordsService userKeywordsService;
	

	public void execute(JobExecutionContext context) throws JobExecutionException {
			
		JobDataMap dataMap = context.getJobDetail().getJobDataMap();
		String username = dataMap.getString("username");
		String email = dataMap.getString("pushemail");
	  	String keyword = dataMap.getString("keyword");
//	  	DateFormat format = new SimpleDateFormat(); 
//	  	Date lastpushdate = null;
//	  	try{
//	  		if (dataMap.getString("lastpushdate") != null )
//	  			if( !dataMap.getString("lastpushdate").isEmpty() )
//	  				lastpushdate = format.parse(dataMap.getString("lastpushdate"));
//		} catch (ParseException e) { 
//	        if ( logger.isInfoEnabled() ) 
//	        	logger.info(e.getStackTrace());
//		}
	  	
        if ( logger.isInfoEnabled() ) 
        	logger.info("FirstPushSendEmailJob execute!!!!!!!!!! email:" + email + ", name:" + username +", key:" + keyword );
	
	  		
        try {
        	String title = "欢迎您的订阅!---SPRING为您打造修身阅读";
        	SendEmail.Send(email, title,QueryByElasticsearch.wrapsingleKeywordHtmlReasult(title, keyword, null,2, 0, 20));
        	userKeywordsService.updateLastpushdate(keyword, username, email, new Date());
        } catch (Exception e) {
			// TODO: handle exception
		}
	     
//	    MongoDao mongoDao = MongoDao.createInstance();
//	    mongoDao.updateLastPushdate(context.getJobDetail().getKey().getName(), newPushDate);

//	    // -- next push trigger -- 	     
//		JobDetail jobDetail = context.getJobDetail();
//	    dataMap.put("lastpushdate", format.format(new Date()));
//		Trigger trigger = context.getTrigger()
//							.getTriggerBuilder()
//							.forJob(jobDetail)
//							.startAt(new Date(new Date().getTime() + 600000))
//							.build();
//		Scheduler schedule = context.getScheduler();
//		
//		try{
//			schedule.deleteJob(context.getJobDetail().getKey());
//			schedule.scheduleJob(jobDetail, trigger);
//	    } catch (SchedulerException se) {
//	    	if (logger.isInfoEnabled()) 
//	    		logger.info(se.getStackTrace());
//	    }
		
	}
}

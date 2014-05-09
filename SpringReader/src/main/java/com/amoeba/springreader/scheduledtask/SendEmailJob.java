package com.amoeba.springreader.scheduledtask;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javassist.expr.NewArray;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.quartz.Job;
import org.quartz.JobExecutionContext;
import org.quartz.JobExecutionException;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Component;

import com.amoeba.springreader.domain.UserAccount;
import com.amoeba.springreader.domain.UserKeyword;
import com.amoeba.springreader.searchengine.QueryByElasticsearch;
import com.amoeba.springreader.service.UserKeywordsService;
import com.amoeba.springreader.service.UserService;

@Component
public class SendEmailJob implements Job {
		
	private static Log logger = LogFactory.getLog("SendEmailJob");
	
	@Autowired(required=true)
	private UserService userService;
	
	@Autowired(required=true)
	private UserKeywordsService userKeywordsService;
	
	

	public void execute(JobExecutionContext context) throws JobExecutionException {
		
        SimpleDateFormat dateFormat2 = new SimpleDateFormat("yyyy/MM/dd");        
        String dateString = dateFormat2.format(new Date()); 

		
		List<UserAccount> userList = userService.findall();
		
		for ( UserAccount user :userList){
			List<UserKeyword> userKeywordsList = userKeywordsService.findByUsername(user.getUsername());
			
			Map<String,String> htmlResultMap = new HashMap<String, String>();
			for (UserKeyword userKeyword : userKeywordsList){
				if (!htmlResultMap.containsKey(userKeyword.getEmail())){
					String htmlResult = QueryByElasticsearch.querySingleKeywordHtmlResult(userKeyword.getName(),
																userKeyword.getLastpushdate(),
																2,
																0,
																10);
					htmlResultMap.put(userKeyword.getEmail(), htmlResult);
				} else {
					
					String htmlResult =  htmlResultMap.get(userKeyword.getEmail())
							+ QueryByElasticsearch.querySingleKeywordHtmlResult(userKeyword.getName(),
									userKeyword.getLastpushdate(),
									2,
									0,
									10);
					htmlResultMap.put(userKeyword.getEmail(), htmlResult);
				}
			}
			
	        if ( logger.isInfoEnabled() ) 
	        	logger.info("DialyPushSendEmailJob execute!!!!!!!!!! ");
		
		  		
	        try {
				for (Map.Entry<String, String> entry : htmlResultMap.entrySet()) {  
				    SendEmail.Send(entry.getKey(),"每日推送!---SPRING为您打造修身阅读",QueryByElasticsearch.wrapKeywordsHtmlReasult(dateString + "最新资讯",entry.getValue()));					
				    userKeywordsService.updateLastpushdateAllKeyword( user.getUsername(), entry.getKey(), new Date());
				}  	        	
			} catch (Exception e) {
				// TODO: handle exception
			}
		}	
		
		
			
//		JobDataMap dataMap = context.getJobDetail().getJobDataMap();
//		String title = dataMap.getString("title");
//		String email = dataMap.getString("pushemail");
//	  	String mailContent = dataMap.getString("html");


//		String username = dataMap.getString("username");
//		String email = dataMap.getString("pushemail");
//	  	String keyword = dataMap.getString("keyword");
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
	  	
//        if ( logger.isInfoEnabled() ) 
//        	logger.info("SendEmailJob execute!!!!!!!!!! email:" + email );// + ", name:" + username +", key:" + keyword
//	
//	  		
//        try {
////        	SendEmail.Send(email,"title",QueryByElasticsearch.getHtmlReasult(keyword, lastpushdate));
//        	SendEmail.Send(email,title,mailContent);
//		} catch (Exception e) {
//			// TODO: handle exception
//		}
			    
//		userKeywordsService.updateLastpushdate(keyword, username, email, new Date());
//	     
////	    MongoDao mongoDao = MongoDao.createInstance();
////	    mongoDao.updateLastPushdate(context.getJobDetail().getKey().getName(), newPushDate);
//
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
//		
	}
}

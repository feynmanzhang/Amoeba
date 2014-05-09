package com.amoeba.ScheduledTasks;

import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.elasticsearch.action.search.SearchResponse;
import org.elasticsearch.client.Client;
import org.elasticsearch.index.query.FilterBuilder;
import org.elasticsearch.index.query.FilterBuilders;
import org.elasticsearch.index.query.QueryStringQueryBuilder;
import org.elasticsearch.search.SearchHit;
import org.elasticsearch.search.SearchHits;
import org.quartz.Job;
import org.quartz.JobDataMap;
import org.quartz.JobDetail;
import org.quartz.JobExecutionContext;
import org.quartz.JobExecutionException;
import org.quartz.Scheduler;
import org.quartz.SchedulerException;
import org.quartz.Trigger;

import com.amoeba.Mongodb.MongoDao;
import com.amoeba.Mongodb.UserBean;

public class SendEmailJob implements Job {
		
	private static Log logger = LogFactory.getLog("SendEmailJob"); 

	public void execute(JobExecutionContext context) throws JobExecutionException {
			
		JobDataMap dataMap = context.getJobDetail().getJobDataMap();
		String email = dataMap.getString(UserBean.UserFields.EMAIL);
	  	String keyword = dataMap.getString(UserBean.UserFields.KEYWORD);
	  	DateFormat format = new SimpleDateFormat(); 
	  	Date lastpushdate = null;
	  	try{
	  		if (dataMap.getString(UserBean.UserFields.LASTPUSHDATE) != null )
	  			if( !dataMap.getString(UserBean.UserFields.LASTPUSHDATE).isEmpty() )
	  				lastpushdate = format.parse(dataMap.getString(UserBean.UserFields.LASTPUSHDATE));
		} catch (ParseException e) { 
	        if ( logger.isInfoEnabled() ) 
	        	logger.info(e.getStackTrace());
		}
	  	
        if ( logger.isInfoEnabled() ) 
        	logger.info("SendEmailJob execute!!!!!!!!!! email:" + email + ", key:" + keyword );
	
	  	
	  	Client client = ElasticSearchOper.getEsClient();
//        QueryBuilder query = QueryBuilders.termQuery("description", keyword); 
       
        QueryStringQueryBuilder query = new QueryStringQueryBuilder(keyword);
        query.analyzer("ik").field("description");
        
        Date newPushDate =  new Date();
        FilterBuilder filter =  FilterBuilders.rangeFilter("timestamp").from(lastpushdate).to(newPushDate);
        SearchResponse response = client.prepareSearch(Application.properties.getProperty("es.rssdata.index"))  
						                .setTypes(Application.properties.getProperty("es.rssdata.type"))  
						                .setQuery(query)
						                .setPostFilter(filter)
						                .setMinScore(1)
						                .setFrom(0)
						                .setSize(10)
						                .execute()  
						                .actionGet();  

		SearchHits shs = response.getHits();  
		for(SearchHit hit : shs){  
	        if ( logger.isInfoEnabled() ) 
	        	logger.info("Sending email:" + email + ", key:" + keyword + ", score:"+hit.getScore()+", title:"+  hit.getSource().get("title")+", timestamp:"+  hit.getSource().get("timestamp"));  	         
	        try {
	        	SendEmail.Send(email,(String)hit.getSource().get("title")+ "[ " + (String)hit.getSource().get("feedname") + " ] "  + "@订阅(" + keyword + ")", 
	        			"<html> <body>" + (String)hit.getSource().get("description") + 
	        			"<br><a href =\"" + (String)hit.getSource().get("link") +"\" target=\"_blank\">点击查看原文</a></body></html>");
//	        	SendEmail.Send(email,"[ PUSH! ]" + (String)hit.getSource().get("title") + "( " + keyword + " )"+ "[ " + (String)hit.getSource().get("feedname") + " ]", 
//	        			"<html> <body>" + (String)hit.getSource().get("description") + "</body></html>");
	        } catch (Exception e) {
	        	if (logger.isInfoEnabled()) 
	        		logger.info(e.getStackTrace());
	        }
		}
	     
	     
	    MongoDao mongoDao = MongoDao.createInstance();
	    mongoDao.updateLastPushdate(context.getJobDetail().getKey().getName(), newPushDate);

	    // -- next push trigger -- 	     
		JobDetail jobDetail = context.getJobDetail();
	    dataMap.put(UserBean.UserFields.LASTPUSHDATE, format.format(newPushDate));
		Trigger trigger = context.getTrigger()
							.getTriggerBuilder()
							.forJob(jobDetail)
							.startAt(new Date(newPushDate.getTime() + Long.parseLong(Application.properties.getProperty("mail.pushrate"))))
							.build();
		Scheduler schedule = context.getScheduler();
		
		try{
			schedule.deleteJob(context.getJobDetail().getKey());
			schedule.scheduleJob(jobDetail, trigger);
	    } catch (SchedulerException se) {
	    	if (logger.isInfoEnabled()) 
	    		logger.info(se.getStackTrace());
	    }
		
	}
}

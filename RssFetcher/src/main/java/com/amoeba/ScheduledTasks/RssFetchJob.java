package com.amoeba.ScheduledTasks;

import java.io.IOException;
import java.net.MalformedURLException;
import java.net.URL;
import java.net.URLConnection;
import java.text.DateFormat;
import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.UUID;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.quartz.Job;
import org.quartz.JobDataMap;
import org.quartz.JobDetail;
import org.quartz.JobExecutionContext;
import org.quartz.JobExecutionException;
import org.quartz.Scheduler;
import org.quartz.SchedulerException;
import org.quartz.Trigger;

import com.amoeba.Mongodb.MongoDao;
import com.amoeba.Mongodb.RssFeedBean;
import com.sun.syndication.feed.rss.Channel;
import com.sun.syndication.feed.synd.SyndEntry;
import com.sun.syndication.feed.synd.SyndFeed;
import com.sun.syndication.io.FeedException;
import com.sun.syndication.io.SyndFeedInput;
import com.sun.syndication.io.XmlReader;


public class RssFetchJob implements Job {
	
	private static Log logger = LogFactory.getLog("RssFetchJob"); 

	public void execute(JobExecutionContext context) throws JobExecutionException {
    	
		//rss feed meta data from JobDataMap
		JobDataMap dataMap = context.getJobDetail().getJobDataMap();
		String feedname = dataMap.getString(RssFeedBean.RssFeedFields.FEEDNAME);
	  	String url = dataMap.getString(RssFeedBean.RssFeedFields.URL);
	  	DateFormat format = new SimpleDateFormat(); 
	  	Date lastDate = null;
	  	Date feedDate =  null;
	  	try{
	  		if (dataMap.getString(RssFeedBean.RssFeedFields.LASTUPDATE) != null )
	  			if( !dataMap.getString(RssFeedBean.RssFeedFields.LASTUPDATE).isEmpty() )
	  				lastDate = format.parse(dataMap.getString(RssFeedBean.RssFeedFields.LASTUPDATE));
		} catch (ParseException e) { 
	        if ( logger.isInfoEnabled() ) 
	        	logger.info(e.getStackTrace());
		}  
		long updateRate = dataMap.getLong(RssFeedBean.RssFeedFields.UPDATERATE);		
		if ( updateRate < 1000 )
			updateRate = Integer.parseInt(Application.properties.getProperty("updaterate"));
		
		
			
        if ( logger.isInfoEnabled() ) 
        	logger.info("RssFetcher execute!!!!!!!!!! name:" + feedname + ", url:" + url + ", updaterate:"+ updateRate);
        
        
        long nextTriggerTime = System.currentTimeMillis() + updateRate;
		
        // Reading feed
        MongoDao mongoDao  = MongoDao.createInstance();
		SyndFeed feed = getFeed(url);
		
        if ( feed != null ) {        	
            feedDate = feed.getPublishedDate();
            if (logger.isDebugEnabled()) 
            	logger.debug("Reading feed from " + url +" , Feed publish date is " + feedDate);
            
            if (lastDate == null || (feedDate != null && feedDate.after(lastDate))) {
                // We have to send results to mongodb
                if (logger.isTraceEnabled()) 
                	logger.trace("Feed is updated : {}" + feedname);
                for (SyndEntry entry : (Iterable<SyndEntry>) feed.getEntries()) {
            		if(entry.getLink() ==  null || entry.getTitle() == null)
            			continue;
            	
            		String uuid = UUID.nameUUIDFromBytes((entry.getLink() + entry.getTitle()).getBytes()).toString();
            		if(!mongoDao.IsUuidExist(uuid))
            			mongoDao.insert(mongoDao.getCollection(Application.properties.getProperty("mongodb.datacolletion")), entry , uuid , feedname);
                } 
                
                lastDate = feedDate;
                mongoDao.updateLastUpdate(feedname, lastDate); // It make little sense. we also can do not do this.
                
            } else {            	
                // Nothing new... Just relax !
                if (logger.isDebugEnabled()) 
                	logger.debug("Nothing new in the feed... Relaxing...");
            }
        		
            
            
            // Use the ttl rss field to auto adjust feed refresh rate
            if ( feed.originalWireFeed() != null && feed.originalWireFeed() instanceof Channel) {
                Channel channel = (Channel) feed.originalWireFeed();
                if (channel.getTtl() > 0) {
                    int ms = channel.getTtl() * 60 * 1000;
                    if (ms != updateRate) {
                        updateRate = ms;
                        if (logger.isInfoEnabled())
                        	logger.info("Auto adjusting update rate with provided ttl: " + channel.getTtl());
                    }
                    
                    // If TTL element is configured in feed , we should synchronous the job trigger. 
                    if (feedDate != null) {
                    	if(feedDate.getTime() + updateRate > System.currentTimeMillis()){  
                    		nextTriggerTime = feedDate.getTime() + updateRate + 60000; // -- 60000 MS to make RSS publish gap --
                    	} else {
                    		nextTriggerTime = System.currentTimeMillis() + 60000;    // -- publishData + TTL < now  --
                    	}
                    		
    				}
                }
                
            }
       
        }
            
		if ( logger.isInfoEnabled() ) 
			logger.info("Job is going to suspend for" + updateRate + " ms");			

		JobDetail jobDetail = context.getJobDetail();
		dataMap.put(RssFeedBean.RssFeedFields.LASTUPDATE, lastDate==null ? "":format.format(lastDate));
		dataMap.put(RssFeedBean.RssFeedFields.UPDATERATE, updateRate);
		Trigger trigger = context.getTrigger()
							.getTriggerBuilder()
							.forJob(jobDetail)
							.startAt( new Date(nextTriggerTime))
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
	
	private SyndFeed getFeed(String url) {
		try {
			URL feedUrl = new URL(url);
			URLConnection openConnection = feedUrl.openConnection();
	        openConnection.addRequestProperty("User-Agent", "RSS River for Elasticsearch (https://github.com/dadoonet/rssriver)"); 
			SyndFeedInput input = new SyndFeedInput();
            input.setPreserveWireFeed(true);
			SyndFeed feed = input.build(new XmlReader(openConnection));
			return feed;
		} catch (MalformedURLException e) {
			logger.error("RSS Url is incorrect : " + url);
		} catch (IllegalArgumentException e) {
			logger.error("Feed from this url is incorrect:" + url);
		} catch (FeedException e) {
			logger.error("Can not parse feed from url:" + url);
		} catch (IOException e) {
			logger.error("Can not read feed from  url:" + url);
		}
		
		return null;
	}
}

package com.amoeba.Deprecated;
//package com.amoeba.Rss;
//
//import java.io.IOException;
//import java.net.MalformedURLException;
//import java.net.URL;
//import java.net.URLConnection;
//import java.util.Date;
//
//import com.amoeba.Mongodb.MongoDao;
//import com.amoeba.ScheduledTasks.Application;
//import com.sun.syndication.feed.rss.Channel;
//import com.sun.syndication.feed.synd.SyndEntry;
//import com.sun.syndication.feed.synd.SyndFeed;
//import com.sun.syndication.io.FeedException;
//import com.sun.syndication.io.SyndFeedInput;
//import com.sun.syndication.io.XmlReader;
//
//
//public class RssParser implements Runnable {
//
//	private String url;
//	private int updateRate;
//	private String feedname;
//    private boolean ignoreTtl;
//    private MongoDao mongoDao;
//
//    public RssParser(RssFeedDefinition feedDefinition,MongoDao mongoDao) {     
//        this.mongoDao = mongoDao;        
//		this.feedname = feedDefinition.getFeedname();
//		this.url = feedDefinition.getUrl();
//		this.updateRate = feedDefinition.getUpdateRate();
//        this.ignoreTtl = feedDefinition.isIgnoreTtl();
//        
//        if (Application.logger.isInfoEnabled()) 
//        	Application.logger.info("creating rss name:" + feedname + ", url:" + url + ", updaterate:"+ updateRate);
//    }
//    
//    @Override
//    public  void run() {
//        while (true) {        	
//			SyndFeed feed = getFeed(url);
//            if (feed != null) {
//                Date feedDate = feed.getPublishedDate();
//                if (Application.logger.isDebugEnabled()) 
//                	Application.logger.debug("Reading feed from {}" + url +"Feed publish date is {}" + feedDate);
//                
//                Date lastDate = mongoDao.getLastUpdate(this.feedname);
//                if (lastDate == null || (feedDate != null && feedDate.after(lastDate))) {
//                    // We have to send results to mongodb
//                    if (Application.logger.isTraceEnabled()) 
//                    	Application.logger.trace("Feed is updated : {}" + this.feedname);
//                    for (SyndEntry entry : (Iterable<SyndEntry>) feed.getEntries()) {
//                    		mongoDao.insert(mongoDao.getCollection(this.feedname), entry);
//                        }                    
//                    mongoDao.updateLastUpdate(this.feedname, feedDate);
//                    
//                } else {
//                    // Nothing new... Just relax !
//                    if (Application.logger.isDebugEnabled()) 
//                    	Application.logger.debug("Nothing new in the feed... Relaxing...");
//                }
//            }
//			try {
//                //Use the ttl rss field to auto adjust feed refresh rate
//                if (!ignoreTtl && feed.originalWireFeed() != null && feed.originalWireFeed() instanceof Channel) {
//                    Channel channel = (Channel) feed.originalWireFeed();
//                    if (channel.getTtl() > 0) {
//                        int ms = channel.getTtl() * 60 * 1000;
//                        if (ms != updateRate) {
//                            updateRate = ms;
//                            if (Application.logger.isInfoEnabled())
//                            	Application.logger.info("Auto adjusting update rate with provided ttl: {} mn" + channel.getTtl());
//                        }
//                    }
//                }
//
//				if (Application.logger.isDebugEnabled()) 
//					Application.logger.debug("Rss river is going to sleep for {} ms" + updateRate);
//				
//				Thread.sleep(updateRate);
//			} catch (InterruptedException e1) {
//				
//			}
//		}
//	}
//
//    
//	private SyndFeed getFeed(String url) {
//		try {
//			URL feedUrl = new URL(url);
//			URLConnection openConnection = feedUrl.openConnection();
//	        openConnection.addRequestProperty("User-Agent", "RSS River for Elasticsearch (https://github.com/dadoonet/rssriver)"); 
//			SyndFeedInput input = new SyndFeedInput();
//            input.setPreserveWireFeed(true);
//			SyndFeed feed = input.build(new XmlReader(openConnection));
//			return feed;
//		} catch (MalformedURLException e) {
//			Application.logger.error("RSS Url is incorrect : [{}]." + url);
//		} catch (IllegalArgumentException e) {
//			Application.logger.error("Feed from [{}] is incorrect." + url);
//		} catch (FeedException e) {
//			Application.logger.error("Can not parse feed from [{}]." + url);
//		} catch (IOException e) {
//			Application.logger.error("Can not read feed from [{}]." + url);
//		}
//		
//		return null;
//	}
//}
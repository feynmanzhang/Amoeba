package com.amoeba.Deprecated;
//package com.amoeba.Rss;
//
//import java.util.ArrayList;
//
//import org.springframework.boot.CommandLineRunner;
//
//import com.amoeba.Mongodb.MongoDao;
//import com.amoeba.ScheduledTasks.Application;
//
//
//public class RssSourceFetcher implements CommandLineRunner{
//
//	public volatile ArrayList<Thread> threads;
//	public int threadNumber = 0;
//	public boolean closed = true;
//	public MongoDao mongoDao;
// 
//	@Override
//	public void run(String... args){
//		closed = false;
//		
//		mongoDao = new MongoDao();
//		ArrayList<RssFeedDefinition> feedsArrayList = mongoDao.findall("rssfeeds");
//		
//        // We create as many Threads as there are feeds
//		threads = new ArrayList<Thread>(feedsArrayList.size());		
//		for (RssFeedDefinition feedDefinition : feedsArrayList) {
//			Thread thread =new Thread(new RssParser(feedDefinition,mongoDao));
//			thread.start();
//			threads.add(thread);
//			threadNumber++;
//		}
//	}
//	
//
//	public void close() {
//		if (Application.logger.isInfoEnabled()) 
//			Application.logger.info("Closing rss fetcher");
//		
//		if (threads != null) {
//			for (Thread thread : threads) {
//				if (thread != null) {
//					thread.interrupt();
//				}
//			}
//		}
//		
//		closed = true;
//		threadNumber = 0;
//		mongoDao.close();
//		
//	}
//
//}

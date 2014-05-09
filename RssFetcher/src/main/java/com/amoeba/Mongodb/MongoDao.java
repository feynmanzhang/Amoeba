package com.amoeba.Mongodb;


import java.util.ArrayList;
import java.util.Date;
import java.util.NoSuchElementException;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.amoeba.ScheduledTasks.Application;
import com.mongodb.BasicDBObject;
import com.mongodb.Cursor;
import com.mongodb.DB;
import com.mongodb.DBCollection;
import com.mongodb.DBCursor;
import com.mongodb.DBObject;
import com.mongodb.MongoClient;
import com.mongodb.MongoException;
import com.sun.syndication.feed.synd.SyndEntry;


public class MongoDao {

	private static Log logger = LogFactory.getLog("MongoDao"); 
	private static MongoDao singleton;
	MongoClient mongoClient = null;
	DB db = null;
		
	public MongoDao(){		
		try {
			mongoClient =  new MongoClient(Application.properties.getProperty("mongodb.host"), 
					Integer.parseInt(Application.properties.getProperty("mongodb.port")));
			db = mongoClient.getDB(Application.properties.getProperty("mongodb.db"));
		} catch (Exception e) {
			if(logger.isErrorEnabled())
				logger.error("mongodb can not open!");
		}
		
	}
	
	public static MongoDao createInstance(){
		if(singleton == null){
			synchronized(MongoDao.class){
				if(singleton == null)
					singleton = new MongoDao();
			}
		}
		
		return singleton;
	}
	
	
	public void close(){
		if(mongoClient != null)
			mongoClient.close();
	}
	

	public DBCollection getCollection(String collectionName){
		if(db == null)
			return null;
		
		return db.getCollection(collectionName);
	}

	
	public void insert(DBCollection coll,SyndEntry entry ,String uuid , String feedname){
		try {
			coll.insert(EntryDBObject.getInstance( entry , uuid , feedname));
		}catch (MongoException e) {
//			if(logger.isErrorEnabled())
//				logger.error("MongoException,insert error" + e.getMessage());
		}
	}
	
	public boolean IsUuidExist(String uuid){
		DBCursor cursor = getCollection(Application.properties.getProperty("mongodb.datacolletion"))
								.find(new BasicDBObject("uuid",uuid));
		
		try {
			if(cursor.hasNext())
				return true;	
		} catch (MongoException e) {
			if(logger.isErrorEnabled())
				logger.error("MongoException,isuuidexist error" + e.getMessage());
		}
		
		return false;
	}
	
	public ArrayList<RssFeedBean> findAllFeeds(String collectionName){
		
		DBCollection coll = getCollection(collectionName);
		
		ArrayList<RssFeedBean> objArrayList =  new ArrayList<RssFeedBean>();
		Cursor cursor = coll.find();

		try {
		   while(cursor.hasNext()) {
		       RssFeedBean rssFeed =  new RssFeedBean();
		       DBObject object = cursor.next();
		       rssFeed.setFeedname(object.get("feedname")==null ? "" : (String)object.get("feedname"));
		       rssFeed.setUrl(object.get("url")==null ? "" : (String)object.get("url"));
		       rssFeed.setUpdateRate(object.get("updaterate")==null ? 0 : (Long)object.get("updaterate")); 
		       rssFeed.setLastUpdate(object.get("lastupdate")==null ? null : (Date)object.get("lastupdate"));
		       objArrayList.add(rssFeed);
		   }
		}catch (NoSuchElementException e) {
			if(logger.isErrorEnabled())
				logger.error("NoSuchElementException" + e.getMessage());
		} finally {
		   cursor.close();
		}
		
		return objArrayList;
	}
	
	public ArrayList<UserBean> findAllUsers(String collectionName){
		
		DBCollection coll = getCollection(collectionName);
		
		ArrayList<UserBean> objArrayList =  new ArrayList<UserBean>();
		Cursor cursor = coll.find();

		try {
		   while(cursor.hasNext()) {
		       UserBean user =  new UserBean();
		       DBObject object = cursor.next();
		       user.setName(object.get("name")==null ? "" : (String)object.get("name"));
		       user.setEmail(object.get("email")==null ? "" : (String)object.get("email"));
		       user.setKeyword(object.get("keyword")==null ? "" : (String)object.get("keyword"));
		       user.setLastPushDate(object.get("lastpushdate")==null ? null : (Date)object.get("lastpushdate"));
		       objArrayList.add(user);
		   }
		}catch (NoSuchElementException e) {
			if(logger.isErrorEnabled())
				logger.error("NoSuchElementException" + e.getMessage());
		} finally {
		   cursor.close();
		}
		
		return objArrayList;
	}
	
//	public Date getLastUpdate(String feedName){
//		Date date = null;
//		
//		DBCursor cursor =getCollection("rssfeeds").find(new BasicDBObject("feedname",feedName));
//		if(!cursor.hasNext())
//			return null;
//		
////		cursor= getCollection(collectionName).find().sort(new BasicDBObject("lastupdate",-1));
////		if(!cursor.hasNext())
////			return null;		
////		
////		cursor= getCollection(collectionName).find().sort(new BasicDBObject("lastupdate",-1)).limit(1);
////		
////		if(cursor.hasNext())
////			date = (Date)cursor.next().get("lastupdate");
//		
//	
//		try {
//				date =  (Date)cursor.next().get("lastupdate");
//			}catch (NoSuchElementException e) {
//				System.out.print("NoSuchElementException" + e.getMessage());
//			} finally {
//			   cursor.close();
//			}
//			
//		return date;
//				
//	}
	
	public void updateLastUpdate(String feedName,Date date){		
		try {
			getCollection(Application.properties.getProperty("mongodb.feedscollection"))
			.update(new BasicDBObject("feedname",feedName),
				new BasicDBObject("$set",new BasicDBObject("lastupdate", date))); 		
		} catch (MongoException e) {
			if(logger.isErrorEnabled())
				logger.error("MongoException,updatelastupdate error" + e.getMessage());
		}
	
		return;
	}

	public void updateLastPushdate(String name,Date date){		
		try {
			getCollection(Application.properties.getProperty("mongodb.userscollection"))
			.update(new BasicDBObject("name",name),
				new BasicDBObject("$set",new BasicDBObject("lastpushdate", date))); 	
		} catch (MongoException e) {
			if(logger.isErrorEnabled())
				logger.error("MongoException,lastpushdate error" + e.getMessage());
		}
	
		return;
	}
		

//	public static void  main(String[] args){
//		MongoDao mongoDao = new MongoDao();
//		DBCollection collection = mongoDao.getCollection("rssfeeds");
//		BasicDBObject object = new BasicDBObject();
//		object.put("feedname", "feed36kr");
//		object.put("url", "http://www.36kr.com/feed/");
//		object.put("updaterate", 30*1000);
//		object.put("ignorettl", true);
//		object.put("lastupdate", new Date(System.currentTimeMillis() + 30*24*3600*1000));
//		collection.insert(object);
//		
//		
//		object = new BasicDBObject();
//		object.put("feedname", "feediheima");
//		object.put("url", "http://www.iheima.com/index.php?m=content&c=rssall&rssid=9");
//		object.put("updaterate", 30*1000);
//		object.put("ignorettl", false);
//		object.put("lastupdate", new Date(System.currentTimeMillis() + 30*24*3600*1000));
//		collection.insert(object);
//			
//		DBCursor cursor = collection.find();
//		try {
//		   while(cursor.hasNext()) {
//		       System.out.println(cursor.next());
//		   }
//		} finally {
//		   cursor.close();
//		}
//
//	}
	
	public static void  main(String[] args){
		MongoDao mongoDao = new MongoDao();
		DBCollection collection = mongoDao.getCollection("users");
		BasicDBObject object = new BasicDBObject();
		object.put("name", "clEvernote21");
		object.put("email", "tiim.81aa21f@m.yinxiang.com");
		object.put("keyword", "产品经理");
		collection.insert(object);
		object = new BasicDBObject();
		object.put("name", "clEvernote22");
		object.put("email", "tiim.81aa21f@m.yinxiang.com");
		object.put("keyword", "趋势");
		collection.insert(object);
		object = new BasicDBObject();
		object.put("name", "clEvernote23");
		object.put("email", "tiim.81aa21f@m.yinxiang.com");
		object.put("keyword", "手游");
		collection.insert(object);
		object = new BasicDBObject();
		object.put("name", "clEvernote24");
		object.put("email", "tiim.81aa21f@m.yinxiang.com");
		object.put("keyword", "投资");
		collection.insert(object);
			
		DBCursor cursor = collection.find();
		try {
		   while(cursor.hasNext()) {
		       System.out.println(cursor.next());
		   }
		} finally {
		   cursor.close();
		}

	}
	
}

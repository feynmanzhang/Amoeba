package com.amoeba.Mongodb;

import java.util.Date;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;

import com.amoeba.Mongodb.RssDataBean.RssDataFields;
import com.mongodb.BasicDBObject;
import com.sun.syndication.feed.synd.SyndEntry;

public class EntryDBObject{
	
	public static Log logger = LogFactory.getLog("EntryDBObject"); 
	 
	public static BasicDBObject getInstance(SyndEntry entry , String uuid , String feedname){		
		
		if(entry.getDescription() == null || entry.getTitle() == null || entry.getLink() == null)
			return null;
		
		BasicDBObject object = new BasicDBObject().append(RssDataFields.UUID, uuid)
											.append(RssDataFields.FEEDNAME, feedname)
											.append(RssDataFields.TIMESTAMP,new Date())
											.append(RssDataFields.LINK,entry.getLink())
					 						.append(RssDataFields.TITLE,entry.getTitle())
									 		.append(RssDataFields.DESCRIPTION,entry.getDescription().getValue());
		
		if(entry.getAuthor() != null)
			object.append(RssDataFields.AUTHOR, entry.getAuthor());		
		if(entry.getPublishedDate() !=  null)
			object.append(RssDataFields.PUBLISHEDDATE,entry.getPublishedDate()); 
		
        if (logger.isDebugEnabled()) 
        	logger.debug(object);
		return object;
	 }
		
	
	static public final class Rss {
		public static final String FEEDNAME = "feedname";
		public static final String AUTHOR = "author";
		public static final String TITLE = "title";
		public static final String DESCRIPTION = "description";
		public static final String LINK = "link";
		public static final String PUBLISHED_DATE = "publisheddate";
		public static final String SOURCE = "source";
		public static final String CATEGORIES = "categories";
		
		public static final String LOCATION = "location";
		static public final class Location {
		    public static final String LAT = "lat";
		public static final String LON = "lon";
		}
		
		public static final String ENCLOSURES = "enclosures";
		static public final class Enclosures {
			public static final String URL = "url";
			public static final String TYPE = "type";
			public static final String LENGTH = "length";
		}
	}
}

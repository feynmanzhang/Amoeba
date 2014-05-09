package com.amoeba.springreader.domain;

public class Article {

	private String uuid;
    private String feedname;
    private String timestamp;
    private String title;
    private String link;
    private String description;
    private String author;
    private String publisheddate;
    
        
    public Article(String uuid, String feedname, String timestamp,
			String title, String link, String description) {
		this.uuid = uuid;
		this.feedname = feedname;
		this.timestamp = timestamp;
		this.title = title;
		this.link = link;
		this.description = description;
	}
	public String getUuid() {
		return uuid;
	}
	public void setUuid(String uuid) {
		this.uuid = uuid;
	}
	public String getFeedname() {
		return feedname;
	}
	public void setFeedname(String feedname) {
		this.feedname = feedname;
	}
	public String getTimestamp() {
		return timestamp;
	}
	public void setTimestamp(String timestamp) {
		this.timestamp = timestamp;
	}
	public String getTitle() {
		return title;
	}
	public void setTitle(String title) {
		this.title = title;
	}
	public String getLink() {
		return link;
	}
	public void setLink(String link) {
		this.link = link;
	}
	public String getDescription() {
		return description;
	}
	public void setDescription(String description) {
		this.description = description;
	}
	public String getAuthor() {
		return author;
	}
	public void setAuthor(String author) {
		this.author = author;
	}
	public String getPublisheddate() {
		return publisheddate;
	}
	public void setPublisheddate(String publisheddate) {
		this.publisheddate = publisheddate;
	}
}

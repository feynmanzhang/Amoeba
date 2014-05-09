package com.amoeba.springreader.domain;

import java.util.Date;

import org.springframework.data.annotation.Id;
import org.springframework.data.mongodb.core.mapping.Document;

@Document
public class UserArticle {
	@Id
	private String id;

	private String articleuuid;
	private String username;
	private Date readdate;
	private String status;
	private String keyword;

	public UserArticle() {

	}
	
	public UserArticle(String articleuuid, String username, Date readdate, String status, String keyword) {
		this.articleuuid = articleuuid;
		this.username = username;
		this.readdate = readdate;
		this.status = status;
		this.keyword = keyword;
	}


	public String getId() {
		return id;
	}

	public void setId(String id) {
		this.id = id;
	}

	public String getArticleuuid() {
		return articleuuid;
	}

	public void setArticleuuid(String articleuuid) {
		this.articleuuid = articleuuid;
	}

	public String getUsername() {
		return username;
	}

	public void setUsername(String username) {
		this.username = username;
	}

	public Date getReaddate() {
		return readdate;
	}

	public void setReaddate(Date readdate) {
		this.readdate = readdate;
	}

	public String getStatus() {
		return status;
	}

	public void setStatus(String status) {
		this.status = status;
	}

	public String getKeyword() {
		return keyword;
	}

	public void setKeyword(String keyword) {
		this.keyword = keyword;
	}

	@Override
	public String toString() {
		return "UserArcticles [id=" + id + ", ]";
	}
}

package com.amoeba.springreader.domain;

import java.util.Date;

import javax.persistence.Id;

import org.springframework.data.mongodb.core.mapping.Document;

@Document
public class UserKeyword {
	
	@Id
	private String id;
	
	private String name;
	private String type;
	private Date createddate;
	private String username;
	private Date lastpushdate;
	private String email;
	
	public UserKeyword(){
		
	}
	
	public UserKeyword( String name, String type, Date createddate,String username,Date lastpushdate,String email) {
		this.name = name;
		this.type = type;
		this.createddate = createddate;
		this.username = username;
		this.lastpushdate = lastpushdate;
		this.email = email;
	}

	public String getId() {
		return id;
	}

	public void setId(String id) {
		this.id = id;
	}

	public String getName() {
		return name;
	}

	public void setName(String name) {
		this.name = name;
	}
	
	public String getType() {
		return type;
	}

	public void setType(String type) {
		this.type = type;
	}

	public Date getCreateddate() {
		return createddate;
	}

	public void setCreateddate(Date createddate) {
		this.createddate = createddate;
	}


	public String getUsername() {
		return username;
	}

	public void setUsername(String username) {
		this.username = username;
	}

	
	public Date getLastpushdate() {
		return lastpushdate;
	}

	public void setLastpushdate(Date lastpushdate) {
		this.lastpushdate = lastpushdate;
	}

	public String getEmail() {
		return email;
	}

	public void setEmail(String email) {
		this.email = email;
	}

}

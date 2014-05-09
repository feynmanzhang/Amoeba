package com.amoeba.Mongodb;

import java.util.Date;

public class UserBean {
    static public final class UserFields {
        public static final String NAME = "name";
        public static final String EMAIL = "email";
        public static final String KEYWORD = "keyword";
        public static final String LASTPUSHDATE = "lastpushdate";
    }
    
    private String name;
    private String email;
    private String keyword;
    private Date lastPushDate;
    

	public Date getLastPushDate() {
		return lastPushDate;
	}
	public void setLastPushDate(Date lastPushDate) {
		this.lastPushDate = lastPushDate;
	}
	public String getName() {
		return name;
	}
	public void setName(String name) {
		this.name = name;
	}
	public String getEmail() {
		return email;
	}
	public void setEmail(String email) {
		this.email = email;
	}
	public String getKeyword() {
		return keyword;
	}
	public void setKeyword(String keyword) {
		this.keyword = keyword;
	}
}

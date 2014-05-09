/*
 * Licensed to David Pilato (the "Author") under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership. Author licenses this
 * file to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 */

package com.amoeba.Mongodb;

import java.util.Date;


/**
 * Define an RSS Feed with source (aka short name), url , lastupdate and updateRate attributes
 *
 */
public class RssFeedBean {
	
    static public final class RssFeedFields {
        public static final String FEEDNAME = "feedname";
        public static final String URL = "url";
        public static final String LASTUPDATE = "lastupdate";
        public static final String UPDATERATE = "updateRate";
    }
	
	private String feedname;
	private String url;
	private long updateRate;
    private Date lastUpdate;
	
	public RssFeedBean() {
	}
	
	public RssFeedBean(String feedname, String url, long updateRate, Date lastUpdate) {
		this.feedname = feedname;
		this.url = url;
		this.updateRate = updateRate;
        this.lastUpdate = lastUpdate;
	}
	
	public String getFeedname() {
		return feedname;
	}
	
	public void setFeedname(String feedname) {
		this.feedname = feedname;
	}
	
	public String getUrl() {
		return url;
	}

	public void setUrl(String url) {
		this.url = url;
	}

	public long getUpdateRate() {
		return updateRate;
	}

	public void setUpdateRate(long updateRate) {
		this.updateRate = updateRate;
	}

	public Date getLastUpdate() {
		return lastUpdate;
	}

	public void setLastUpdate(Date lastUpdate) {
		this.lastUpdate = lastUpdate;
	}
}

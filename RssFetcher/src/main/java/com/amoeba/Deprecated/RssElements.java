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

package com.amoeba.Deprecated;


public class RssElements {
    static public final class Rss {
        public static final String FEEDNAME = "feedname";
        public static final String AUTHOR = "author";
        public static final String TITLE = "title";
        public static final String DESCRIPTION = "description";
        public static final String LINK = "link";
        public static final String PUBLISHED_DATE = "publishedDate";
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

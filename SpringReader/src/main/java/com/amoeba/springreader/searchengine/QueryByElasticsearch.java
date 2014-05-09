package com.amoeba.springreader.searchengine;

import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
import org.elasticsearch.action.search.SearchResponse;
import org.elasticsearch.client.Client;
import org.elasticsearch.index.query.FilterBuilder;
import org.elasticsearch.index.query.FilterBuilders;
import org.elasticsearch.index.query.QueryStringQueryBuilder;
import org.elasticsearch.search.SearchHit;
import org.elasticsearch.search.SearchHits;
import org.elasticsearch.search.sort.SortBuilders;
import org.elasticsearch.search.sort.SortOrder;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.PageImpl;
import org.springframework.data.domain.Pageable;

import com.amoeba.springreader.domain.Article;
import com.amoeba.springreader.repository.ElasticSearchOper;
import com.amoeba.springreader.scheduledtask.SendEmail;
import com.amoeba.springreader.until.CommonConverter;

public class QueryByElasticsearch {
	
	private static Log logger = LogFactory.getLog("QueryByElasticsearch");
	
	public static Page<Article> queryKeywordResultInPage(String keyword , Pageable pageable) {
	  	Client client = ElasticSearchOper.getEsClient();
//      QueryBuilder query = QueryBuilders.termQuery("description", keyword); 
     
	  	QueryStringQueryBuilder query = new QueryStringQueryBuilder(keyword);
	  	query.analyzer("ik").field("description");
      
	  	//  Date newPushDate =  new Date();
	  	//  FilterBuilder filter =  FilterBuilders.rangeFilter("timestamp").from(lastpushdate).to(newPushDate);
	  	SearchResponse response = client.prepareSearch("mongoindex")  
						                .setTypes("rssdata")  
						                .setQuery(query)
						           //     .setPostFilter(filter)
						                .setMinScore(2)
						                .setFrom(pageable.getOffset())
						                .setSize(pageable.getPageSize())
						                .addSort(SortBuilders.fieldSort("timestamp").order(SortOrder.DESC))
						                .execute()  
						                .actionGet();  

		SearchHits shs = response.getHits();
		List<Article> list = new ArrayList<Article>();
		for(SearchHit hit : shs){		
			try {
				list.add(new Article((String)hit.getSource().get("uuid"),
						(String)hit.getSource().get("feedname"),
						CommonConverter.toDateStringFromIso((String)hit.getSource().get("timestamp")),
						(String)hit.getSource().get("title"),
						(String)hit.getSource().get("link"),
						subString(CommonConverter.delTagsFContent((String)hit.getSource().get("description")),140)));
		    } catch (Exception e) {
		    	e.getMessage();
		    }
		}		
		
		Page<Article> uiBeans = new PageImpl<Article>(list, pageable,shs.getTotalHits());
		
		return uiBeans;
	}
	
	public static String querySingleKeywordHtmlResult(String keyword,Date lastpushdate, float minscore,int from, int size){
	  	Client client = ElasticSearchOper.getEsClient();
//      QueryBuilder query = QueryBuilders.termQuery("description", keyword); 
     
      QueryStringQueryBuilder query = new QueryStringQueryBuilder(keyword);
      query.analyzer("ik").field("description");
      
      Date newPushDate =  new Date();
      FilterBuilder filter =  FilterBuilders.rangeFilter("timestamp").from(lastpushdate).to(newPushDate);
      SearchResponse response = client.prepareSearch("mongoindex")  
						                .setTypes("rssdata")  
						                .setQuery(query)
						                .setPostFilter(filter)
						                .setMinScore(minscore)
						                .setFrom(from)
						                .setSize(size)
						                .addSort(SortBuilders.fieldSort("timestamp").order(SortOrder.DESC))
						                .execute()  
						                .actionGet();  

		SearchHits shs = response.getHits();  
		String resultString = new String();
		for(SearchHit hit : shs){  
	        resultString += "<li><a href=\""+ (String)hit.getSource().get("link") +"\" target=\"_blank\">" +(String)hit.getSource().get("title") 
	        		+ "</a><br/><font>" +subString(CommonConverter.delTagsFContent((String)hit.getSource().get("description")),140) + "</font><p>" + (String)hit.getSource().get("feedname") 
	        		+" • "+CommonConverter.toDateStringFromIso((String)hit.getSource().get("timestamp")) + "</p></li>";		
		}
		
		
		resultString = "<div class=\"mail-txt\"><div class=\"mail-titile\">"+keyword+ "("+(shs.getTotalHits()>size?size:shs.getTotalHits()) + "条内容更新)" + "</span></div><ul>"
						+ resultString + "</ul></div>";
		
		
		return resultString;
	}
	
	public static String subString(String string, int length){
		if(string.length() <= length)
			return string;
		
		return string.substring(0,length);
	}
	
	public static String wrapsingleKeywordHtmlReasult(String title,String keyword,Date lastpushdate, float minscore,int from, int size){
		String resultString = querySingleKeywordHtmlResult(keyword,lastpushdate,minscore,from,size);
				
		resultString = "<!DOCTYPE html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /><title>邮件内容</title><style type=\"text/css\">body{font-family: Helvetica,\"Microsoft YaHei\",Arial,sans-serif;}ul,li{margin:0;padding:0;list-style:none;}.mail{ margin:0 0; overflow:hidden;}.mail-top{ height:70px; line-height:70px; color:#444; font-size:20px; font-weight:700;}.mail-titile{height:40px; background:#f5f5f5; line-height:40px; color:#444; font-weight:bold; padding-left:10px;}.mail-txt{ margin-bottom:30px; overflow:hidden;}.mail-line{border-bottom:1px solid #e4e4e4; padding-bottom:30px; margin-bottom:10px;}.more{margin-bottom:30px;} .more a{color:#2d64b3;}.mail li{margin-top:25px; padding-right:15px;}.mail font{font-size:14px; line-height:26px; color:#666;} .right-sidebar p ,.mail p{color:#999;font-size:14px;margin:0;}.mail li a{font-size:16px; color:#2d64b3; font-weight:bold; line-height:24px;text-decoration:none;}</style></head><body><div class=\"mail\"><div class=\"mail-top\">"
						+title +"</div>" + resultString + "<div class=\"more\"><a href=\"http://106.186.120.119:8080/SpringReader/\">订阅更多内容</a></div></div></body></html>";
		
		
		return resultString;
	}
	
	public static String wrapKeywordsHtmlReasult(String title,String htmlString){
				
		String resultString  = "<!DOCTYPE html><head><meta http-equiv=\"Content-Type\" content=\"text/html; charset=utf-8\" /><title>邮件内容</title><style type=\"text/css\">body{font-family: Helvetica,\"Microsoft YaHei\",Arial,sans-serif;}ul,li{margin:0;padding:0;list-style:none;}.mail{ margin:0 0; overflow:hidden;}.mail-top{ height:70px; line-height:70px; color:#444; font-size:20px; font-weight:700;}.mail-titile{height:40px; background:#f5f5f5; line-height:40px; color:#444; font-weight:bold; padding-left:10px;}.mail-txt{ margin-bottom:30px; overflow:hidden;}.mail-line{border-bottom:1px solid #e4e4e4; padding-bottom:30px; margin-bottom:10px;}.more{margin-bottom:30px;} .more a{color:#2d64b3;}.mail li{margin-top:25px; padding-right:15px;}.mail font{font-size:14px; line-height:26px; color:#666;} .right-sidebar p ,.mail p{color:#999;font-size:14px;margin:0;}.mail li a{font-size:16px; color:#2d64b3; font-weight:bold; line-height:24px;text-decoration:none;}</style></head><body><div class=\"mail\"><div class=\"mail-top\">"
						+title +"</div>" + htmlString + "<div class=\"more\"><a href=\"http://106.186.120.119:8080/SpringReader/\">订阅更多内容</a></div></div></body></html>";
				
		return resultString;
	}
	
	
    public static void main(String [] args)
    {

    	try {
    		SendEmail.Send( "15959013445@139.com", "4月11日文章精选（12期）", wrapsingleKeywordHtmlReasult("苹果","苹果",null,5,0,20));
    	} catch (Exception e) {
    		// TODO: handle exception
    	}
    }
}

package com.amoeba.Deprecated;

import org.elasticsearch.action.search.SearchResponse;
import org.elasticsearch.client.Client;
import org.elasticsearch.client.transport.TransportClient;
import org.elasticsearch.common.settings.ImmutableSettings;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.transport.InetSocketTransportAddress;
import org.elasticsearch.index.query.QueryStringQueryBuilder;
import org.elasticsearch.search.SearchHit;
import org.elasticsearch.search.SearchHits;

public class EsTest {
    public static void main(String[] argv){  
        Settings settings = ImmutableSettings.settingsBuilder()  
                //指定集群名称  
                .put("cluster.name", "elasticsearch")  
                //探测集群中机器状态  
                .put("client.transport.sniff", false).build();  
        /* 
         * 创建客户端，所有的操作都由客户端开始，这个就好像是JDBC的Connection对象 
         * 用完记得要关闭 
         */  
        Client client = new TransportClient(settings)  
        .addTransportAddress(new InetSocketTransportAddress("localhost", 9300));  
        //在这里创建我们要索引的对象  
//        GetResponse response = client.prepareGet("mongoindex", "feed36kr", "530e9c9894ed4a94549f3d94")  
//                .execute().actionGet();  
//        System.out.println("response.getId():"+response.getId());  
//        System.out.println("response.getSourceAsString():"+response.getSourceAsString());  
        
        
//    //    QueryBuilder query1 = QueryBuilders.termsQuery("description", "苹果","三星","iphone");
//      // QueryBuilder query1 = QueryBuilders.termQuery("description", "筹资");
//        QueryStringQueryBuilder query1 = new QueryStringQueryBuilder("互联网  创业 金融 ");
//        query1.analyzer("ik").field("description");
// //       QueryBuilder query2 = QueryBuilders.termQuery("description", "三星");
// //       QueryBuilder query3 = QueryBuilders.matchQuery("timestamp", "2014-03-12T06:58:31.569Z");
////        QueryBuilder query4 =  QueryBuilders.rangeQuery("timestamp").from(new DateTime("2014-03-11T06:38:26.058Z")).to(new DateTime("2014-03-14T14:30:24.474Z"));
////        FilterBuilder filter =  FilterBuilders.rangeFilter("timestamp").from(new DateTime("2014-03-12T06:38:26.058Z")).to(new DateTime("2014-03-12T14:22:25.350Z"));
// //       DateTime dateTime = new DateTime( "2013-10-21T01:34:04.808Z" );
//        SearchResponse response = client.prepareSearch(Application.properties.getProperty("es.rssdata.index"))  
//						                .setTypes(Application.properties.getProperty("es.rssdata.type"))  
//						                //设置查询条件,  
//						                .setQuery(query1)
//						                
//						    //            .setQuery(query2)
//						   //             .setQuery(query4)
//						                 
//						   //             .setPostFilter(filter)
//						                .setFrom(0)
//						                .setSize(10)
//						     //           .setMinScore(2)
//						                .execute()  
//						                .actionGet();  
	  	QueryStringQueryBuilder query = new QueryStringQueryBuilder("三星");
	  	query.analyzer("ik").field("description");
      
	  	//  Date newPushDate =  new Date();
	  	//  FilterBuilder filter =  FilterBuilders.rangeFilter("timestamp").from(lastpushdate).to(newPushDate);
	  	SearchResponse response = client.prepareSearch("mongoindex")  
						                .setTypes("rssdata")  
						                .setQuery(query)
						           //     .setPostFilter(filter)
						                .setMinScore(2)
						                .setFrom(0)
						                .setSize(150)
						                .execute()  
						                .actionGet();  
        
//      /** 
//      * SearchHits是SearchHit的复数形式，表示这个是一个列表 
//      */  
	     SearchHits shs = response.getHits();  
	     int i=0;
	     for(SearchHit hit : shs){  
	         System.out.println("分数(score):"+hit.getScore()+", 业务描述(desc):"+  
	                 hit.getSource().get("title"));  
	         i++;
	     }  
        client.close();  
        System.out.println("searched!" + i);
    }  
}

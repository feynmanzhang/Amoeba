package com.amoeba.springreader.repository;

import org.elasticsearch.client.Client;
import org.elasticsearch.client.transport.TransportClient;
import org.elasticsearch.common.settings.ImmutableSettings;
import org.elasticsearch.common.settings.Settings;
import org.elasticsearch.common.transport.InetSocketTransportAddress;
import org.springframework.stereotype.Repository;

@Repository
public class ElasticSearchOper {
	
	private static Client singleton;
	
	public static Client ElasticSearchOperInit(){
        Settings settings = ImmutableSettings.settingsBuilder()  
							                .put("cluster.name", "elasticsearch")  
							                .put("client.transport.sniff", false).build();  

        Client client = new TransportClient(settings)  
        					.addTransportAddress(new InetSocketTransportAddress("localhost", 9300));  
        
        return client;
	}

	
	public static Client getEsClient(){
		if(singleton == null){
			synchronized(ElasticSearchOper.class){
				if(singleton == null)
					singleton = ElasticSearchOperInit();
			}
		}
		
		return singleton;
	}
	
	public static void close(){
		if(singleton ==  null)
			return;
		
		singleton.close();
	}
	
}

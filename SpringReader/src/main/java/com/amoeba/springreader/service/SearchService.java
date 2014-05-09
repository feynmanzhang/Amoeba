package com.amoeba.springreader.service;

import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.stereotype.Service;

import com.amoeba.springreader.domain.Article;
import com.amoeba.springreader.searchengine.QueryByElasticsearch;

@Service
public class SearchService {


	
	
	public Page<Article> findAll(String keyword , Pageable pageable) {
		
		return QueryByElasticsearch.queryKeywordResultInPage(keyword, pageable);
	}
	
	public String subString(String string, int length){
		if(string.length() <= length)
			return string;
		
		return string.substring(0,length);
	}
		
	public Article findById(String id){
			
			return null;
	}
}	
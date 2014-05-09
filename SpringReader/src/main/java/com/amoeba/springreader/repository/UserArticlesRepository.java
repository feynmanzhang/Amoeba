package com.amoeba.springreader.repository;

import java.util.List;

import org.springframework.data.mongodb.repository.MongoRepository;
import org.springframework.data.repository.PagingAndSortingRepository;

import com.amoeba.springreader.domain.UserArticle;

public interface UserArticlesRepository extends MongoRepository<UserArticle, String>,
		PagingAndSortingRepository<UserArticle, String> {
	List<UserArticle> findByUsername(String username);
	
	List<UserArticle> findByUsernameAndKeyword(String username,String keyword);

	UserArticle findById(String id);
}

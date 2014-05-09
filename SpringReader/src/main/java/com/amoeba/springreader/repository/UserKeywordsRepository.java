package com.amoeba.springreader.repository;


import java.util.List;

import org.springframework.data.mongodb.repository.MongoRepository;
import org.springframework.data.repository.PagingAndSortingRepository;

import com.amoeba.springreader.domain.UserKeyword;

public interface UserKeywordsRepository extends MongoRepository<UserKeyword, String>,
		PagingAndSortingRepository<UserKeyword, String> {
	
	List<UserKeyword> findByUsernameAndType(String username,String type);
	
	List<UserKeyword> findByUsername(String username);
	
	List<UserKeyword> findByUsernameAndEmail(String username, String email);
	
	UserKeyword findByUsernameAndNameAndEmail(String username, String name, String email);

	UserKeyword findById(String id);
}

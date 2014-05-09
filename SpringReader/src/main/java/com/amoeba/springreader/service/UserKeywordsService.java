package com.amoeba.springreader.service;

import java.util.Calendar;
import java.util.Date;
import java.util.List;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Service;

import com.amoeba.springreader.domain.UserKeyword;
import com.amoeba.springreader.repository.UserKeywordsRepository;

@Service
public class UserKeywordsService {

	@Autowired
	private UserKeywordsRepository userKeywordsRepo;
	
//	@Autowired
//	private MongoTemplate mongoTemplate;


	public boolean create(String keyword, String username,String email) {
		
		if(userKeywordsRepo.findByUsernameAndNameAndEmail(keyword,username,email) == null){
			userKeywordsRepo.save(new UserKeyword(keyword,"",Calendar.getInstance().getTime(),username,null,email));
		}
		
		return true;
	}
	
	public boolean delete(String keyword, String username,String email) {
		
		UserKeyword userKeyword = userKeywordsRepo.findByUsernameAndNameAndEmail(username,keyword,email);
		if( userKeyword != null){
			userKeywordsRepo.delete(userKeyword);
		}
		
		return true;
	}
	
	public boolean delete(String id) {
		
		userKeywordsRepo.delete(id);		
		return true;
	}
	
	public boolean update(String keywordSoure,String keywordTarget, String username, String email) {
		
//		Query q = new Query(Criteria.where("username").is(username).and("name").is(keywordSoure));
//		Update updateQ = new Update().set("name",keywordTarget);
//		mongoTemplate.updateFirst(q, updateQ, UserKeyword.class);
		UserKeyword userKeyword = userKeywordsRepo.findByUsernameAndNameAndEmail(username,keywordSoure,email);
		if( userKeyword != null){
			userKeywordsRepo.delete(userKeyword);
			userKeyword.setName(keywordTarget);
			userKeywordsRepo.save(userKeyword);
		}
		return true;
	}
	
	public boolean updateLastpushdate(String keyword,String username, String email, Date lastpushdate){
//		Query q = new Query(Criteria.where("username").is(username).and("name").is(keyword));
//		Update updateQ = new Update().set("lastpushdate",lastpushdate);
//		mongoTemplate.updateFirst(q, updateQ, UserKeyword.class);
		
		UserKeyword userKeyword = userKeywordsRepo.findByUsernameAndNameAndEmail(username,keyword,email);
		if( userKeyword != null){
			userKeywordsRepo.delete(userKeyword);
			userKeyword.setLastpushdate(lastpushdate);
			userKeywordsRepo.save(userKeyword);
		}
		return true;
	}
	
	public boolean updateLastpushdateAllKeyword(String username, String email, Date lastpushdate){
//		Query q = new Query(Criteria.where("username").is(username).and("name").is(keyword));
//		Update updateQ = new Update().set("lastpushdate",lastpushdate);
//		mongoTemplate.updateFirst(q, updateQ, UserKeyword.class);
		
		List<UserKeyword> userKeywordList = userKeywordsRepo.findByUsernameAndEmail(username,email);
		for (UserKeyword userKeyword :userKeywordList){
			userKeywordsRepo.delete(userKeyword);
			userKeyword.setLastpushdate(lastpushdate);
			userKeywordsRepo.save(userKeyword);
		}
		return true;
	}

	public List<UserKeyword> findByUsernameAndType(String name,String type) {
		return userKeywordsRepo.findByUsernameAndType(name,type);
	}

	public List<UserKeyword> findByUsername(String name) {
		return userKeywordsRepo.findByUsername(name);
	}
}

package com.amoeba.springreader.service;

import javax.annotation.PostConstruct;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.context.annotation.Profile;
import org.springframework.data.mongodb.core.MongoOperations;
import org.springframework.security.crypto.password.PasswordEncoder;
import org.springframework.stereotype.Component;

import com.amoeba.springreader.domain.Role;

@Component
@Profile(value="PROD")
public class PushInitializer {
	
	private final Logger logger = LoggerFactory.getLogger(getClass());
	
	@Autowired private MongoOperations operations;
	
	@Autowired private UserService userService;
	
    @Autowired private PasswordEncoder encoder; 
    
    @Autowired protected DbService dbService;
	
	@PostConstruct
	public void init() {
//		String demoPasswordEncoded = encoder.encode("demo");
//		logger.debug("initializing data, demo password encoded: {}", demoPasswordEncoded);
//		
//		//clear all collections, but leave indexes intact
//		dbService.cleanUp();
//		
//		//establish roles
//		operations.insert(new Role("ROLE_USER"), "role");
//		operations.insert(new Role("ROLE_ADMIN"), "role");		
	}
}



//package com.amoeba.springreader.service;
//
//import java.util.List;
//
//import javax.annotation.PostConstruct;
//
//import org.slf4j.Logger;
//import org.slf4j.LoggerFactory;
//import org.springframework.beans.factory.annotation.Autowired;
//import org.springframework.context.annotation.Profile;
//import org.springframework.stereotype.Component;
//
//import com.amoeba.springreader.domain.UserAccount;
//import com.amoeba.springreader.domain.UserKeyword;
//import com.amoeba.springreader.scheduledtask.SendEmailScheduler;
//
//@Component
//@Profile(value="PROD")
//public class PushInitializer {
//	
//	private final Logger logger = LoggerFactory.getLogger(getClass());
//	
//	@Autowired private UserService userService;
//	
//    @Autowired private UserKeywordsService userKeywordsService; 
//    
//    @Autowired private SendEmailScheduler sendEmailScheduler;
//	
//	@PostConstruct
//	public void init() {
//		
//		List<UserAccount> userAccountsList = userService.findall();
//		
//		for(UserAccount userAccount : userAccountsList){
//			List<UserKeyword> userKeywordList = userKeywordsService.findByUsername(userAccount.getUsername());
//			for(UserKeyword userKeyword : userKeywordList){
//				sendEmailScheduler.createTask(userAccount.getUsername(), userAccount.getPushemail(), 
//											userKeyword.getName(), userKeyword.getLastpushdate());
//			}
//		}
//		
//		
//	}
//}

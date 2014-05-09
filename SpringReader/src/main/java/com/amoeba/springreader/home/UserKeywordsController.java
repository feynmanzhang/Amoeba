package com.amoeba.springreader.home;

import java.security.Principal;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.ResponseBody;

import com.amoeba.springreader.scheduledtask.SendEmailScheduler;
import com.amoeba.springreader.service.UserKeywordsService;
import com.amoeba.springreader.service.UserService;

@Controller
@RequestMapping("/keyword")
public class UserKeywordsController {

	@Autowired
	UserKeywordsService userKeywordsService;
	
	@Autowired
	UserService userService;
	
	@Autowired
	SendEmailScheduler sendEmailScheduler;


	@RequestMapping("/get")
	public @ResponseBody String getKeywords( Principal principal) {
		return "";
	}


		
//		String[] keywordsArray = keywords.split(",");   	
////    	String userKeywords = new String();
////    	for(String str : keywordsArray){
////    		userKeywords += EncodeConverter.encodeConverter(str) + ","; 
////    	}
////    	if(!userKeywords.isEmpty())
////    		userKeywords = userKeywords.substring(0,userKeywords.length() - 1);
//		List<UserKeyword> userKeywordList = userKeywordsService.findByUsername(userDetails.getUsername());
//		List<String> keywordList = new ArrayList<String>();
//		for(UserKeyword userKeyword : userKeywordList){
//			keywordList.add(userKeyword.getName());
//		}
//		
//		
//		String userKeywords = new String();
//		
//		for(String keyword : keywordsArray){
//			 String string = EncodeConverter.encodeConverter(keyword);
//			 if(string.isEmpty())
//				 continue;
//			 
//			 boolean bExit = false;
//			 for(String keywordstr : keywordList){
//				 if(keywordstr.equals(string)){
//					 bExit = true;
//				 }
//			 }
//			 
//			 if(!bExit){
//				 userKeywordsService.create(string,userAccount.getUsername());
//				 sendEmailScheduler.createTask(userAccount.getUsername(), 
//														 userAccount.getPushemail(), 
//														 keyword, 
//														 null);
//			 } else {
//				 keywordList.remove(string);
//			 }
//
//			 userKeywords += string + ","; 
//		}
//		
//    	if(!userKeywords.isEmpty())
//    		userKeywords = userKeywords.substring(0,userKeywords.length() - 1);
//		
//		for(String keyword :keywordList){
//				userKeywordsService.delete(keyword, userDetails.getUsername());
//				sendEmailScheduler.deleteTask(userDetails.getUsername(), keyword);
//		}
//		
//    	model.addAttribute("userkeywords", userKeywords);
//    	model.addAttribute("currentuser", userAccount);
//		
//    	return "/index";
//	}

}
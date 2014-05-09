package com.amoeba.springreader.home;

import java.security.Principal;
import java.util.List;

import javax.servlet.http.HttpServletRequest;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.security.core.context.SecurityContextHolder;
import org.springframework.security.core.userdetails.UserDetails;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;

import com.amoeba.springreader.domain.UserAccount;
import com.amoeba.springreader.domain.UserKeyword;
import com.amoeba.springreader.scheduledtask.SendEmailScheduler;
import com.amoeba.springreader.service.UserKeywordsService;
import com.amoeba.springreader.service.UserService;

@Controller
public class HomeController {
	
//	@RequestMapping(value = "/", method = RequestMethod.GET)
//	public String index(Principal principal) {
//		return principal != null ? "home/homeSignedIn" : "home/homeNotSignedIn";
//	}
	
	@Autowired
	private UserKeywordsService userKeywordsService;
	
	@Autowired
	private UserService userService;
	
	@Autowired
	private SendEmailScheduler sendEmailScheduler;
	
    @RequestMapping(value={"/","/index"})
    public String root(Model model) {
    	
    	model.addAttribute(new SignupForm());
        return "/index";
    }

    @RequestMapping("/subscribe")
    public String subscribe(Model model,Principal principal) {
//    	if(principal == null)
//    		return "";//TODO
//    	
//    	UserDetails userDetails = (UserDetails) SecurityContextHolder.getContext().getAuthentication().getPrincipal();
//    	UserAccount userAccount = userService.findByUsername(userDetails.getUsername());
//    	
//    	List<UserKeyword> userKeywordList = userKeywordsService.findByUsername(userDetails.getUsername());
//    	
//    	String userKeywords = new String();
//    	for(UserKeyword uk : userKeywordList){
//    		userKeywords += uk.getName() + ","; 
//    	}
//    	if(!userKeywords.isEmpty())
//    		userKeywords = userKeywords.substring(0,userKeywords.length() - 1);
//    	
//    	model.addAttribute("userkeywords", userKeywords);
//    	model.addAttribute("currentuser", userAccount);
        return "subscribe";
    } 
    
    @RequestMapping("/manage")
    public String manage(Model model,Principal principal) {
    	
		UserDetails userDetails = (UserDetails) SecurityContextHolder.getContext().getAuthentication().getPrincipal();		
		
    	UserAccount userAccount = userService.findByUsername(userDetails.getUsername());      	
    	List<UserKeyword> userKeywordList = userKeywordsService.findByUsername(userDetails.getUsername());    	
    	
    	model.addAttribute("userkeywords", userKeywordList);
    	model.addAttribute("currentuser",userAccount);

       	return "manage";
    }
    
	@RequestMapping(value = "/createsubscribe", method = RequestMethod.GET, produces = "application/json; charset=utf-8")	
	public @ResponseBody String createsubscribe(@RequestParam String keyword,@RequestParam String email, Model model){
		if(keyword.isEmpty() || email.isEmpty())
			return "{\"success\": false,\"message\":\"关键词或邮箱不能为空\"}";
		
    	try {		    		  		    	
	    	if (keyword.equals(new String(keyword.getBytes("ISO8859-1"),"ISO8859-1"))) {
	    		keyword=new String(keyword.getBytes("ISO8859-1"),"UTF-8");  
			}
	    	else if(keyword.equals(new String(keyword.getBytes("UTF-8"),"UTF-8"))){
	    		keyword=new String(keyword.getBytes("UTF-8"),"UTF-8");  
	    	}
	    	else if(keyword.equals(new String(keyword.getBytes("GBK"),"GBK"))){
	    		keyword=new String(keyword.getBytes("GBK"),"UTF-8");  
	    	}	    	
    	} catch (Exception e) {
			// TODO: handle exception
    		e.printStackTrace();
		}
		
		UserDetails userDetails = (UserDetails) SecurityContextHolder.getContext().getAuthentication().getPrincipal();
		UserAccount userAccount = userService.findByUsername(userDetails.getUsername());
		
		List<UserKeyword> userKeywordList = userKeywordsService.findByUsername(userDetails.getUsername());
		
		for(UserKeyword userKeyword : userKeywordList){
			if (userKeyword.getName().equals(keyword) && 
					userKeyword.getEmail().equals(email)){
				return  "{ \"success\": false,\"message\":\"该订阅内容已经存在\"}";
			}
		}
		
		userKeywordsService.create(keyword,userDetails.getUsername(),email);
		sendEmailScheduler.createFirstPushTask(userAccount.getUsername(), email, keyword);
		return "{\"success\": true,\"message\" :\"订阅成功\"}";
	}
    
	
	@RequestMapping(value = "/deletesubscribe", method = RequestMethod.POST, produces = "application/json; charset=utf-8")	
	public @ResponseBody String deletesubscribe(@RequestParam String ids, Model model){
		if(ids.isEmpty() )
			return "{ \"success\": \"false\" }";
		
		String[] idArray = ids.split(",");   	

		for(String id : idArray){
			userKeywordsService.delete(id);
		}
		
		return  "{ \"success\": \"true\" }";
	}
	
//	@RequestMapping(value = "/createsubscribe", method = RequestMethod.POST)
//	public String createSubscribePost(@Valid @ModelAttribute SubscribeForm subscribeForm, Model model){
//		System.out.println("allasdj");
//		
//		UserDetails userDetails = (UserDetails) SecurityContextHolder.getContext().getAuthentication().getPrincipal();
//		UserAccount userAccount = userService.findByUsername(userDetails.getUsername());
//		
//		List<UserKeyword> userKeywordList = userKeywordsService.findByUsername(userDetails.getUsername());
//		
//		for(UserKeyword userKeyword : userKeywordList){
//			if (userKeyword.getName().equals(subscribeForm.getKeyword()) && 
//					userKeyword.getEmail().equals(subscribeForm.getEmail())){
//				return "redirect:/manage";
//			}
//		}
//		
//		 userKeywordsService.create(subscribeForm.getKeyword(),userDetails.getUsername(),subscribeForm.getEmail());
//		 sendEmailScheduler.createFirstPushTask(userAccount.getUsername(), 
//				 								subscribeForm.getEmail(), 
//												 subscribeForm.getKeyword());
//		return "redirect:/manage";
//	}
        
    
//    @RequestMapping("/setting")
//    public String setting(Model model,Principal principal) {
//    	if(principal == null)
//    		return "";
//    	
//    	UserDetails userDetails = (UserDetails) SecurityContextHolder.getContext().getAuthentication().getPrincipal();
//    	UserAccount accout = userService.findByUsername(userDetails.getUsername());   	
//    	model.addAttribute("currentuser",accout);
//    	
//        return "setting";
//    } 
    
	@RequestMapping(value = "/setting/post")
	public String view(@RequestParam String pushemail,Model model, Principal principal){
//    	if(principal == null)
//    		return "";
//    	
//		UserDetails userDetails = (UserDetails) SecurityContextHolder.getContext().getAuthentication().getPrincipal();		
//		userService.updatePushemail(userDetails.getUsername(),pushemail);
//		
//    	UserAccount userAccount = userService.findByUsername(userDetails.getUsername());      	
//    	List<UserKeyword> userKeywordList = userKeywordsService.findByUsername(userDetails.getUsername());    	
//    	String userKeywords = new String();
//    	for(UserKeyword userKeyword : userKeywordList){
//    		userKeywords = userKeyword.getName() + ",";
//    		sendEmailScheduler.deleteTask(userDetails.getUsername(), userKeyword.getName());
//			sendEmailScheduler.createTask(userAccount.getUsername(), userAccount.getPushemail(), 
//					userKeyword.getName(), userKeyword.getLastpushdate());
//    	}
//    	if(!userKeywords.isEmpty())
//    		userKeywords = userKeywords.substring(0,userKeywords.length() - 1);
//    	
//    	model.addAttribute("userkeywords", userKeywords);
//    	model.addAttribute("currentuser",userAccount);

		return "/index";
	
	}

    /** Error page. */
    @RequestMapping("/error")
    public String error(HttpServletRequest request, Model model) {
        model.addAttribute("errorCode", "Error " + request.getAttribute("javax.servlet.error.status_code"));
        Throwable throwable = (Throwable) request.getAttribute("javax.servlet.error.exception");
        StringBuilder errorMessage = new StringBuilder();
        errorMessage.append("<ul>");
        while (throwable != null) {
            errorMessage.append("<li>").append(escapeTags(throwable.getMessage())).append("</li>");
            throwable = throwable.getCause();
        }
        errorMessage.append("</ul>");
        model.addAttribute("errorMessage", errorMessage.toString());
        return "error";
    }

    /** Substitute 'less than' and 'greater than' symbols by its HTML entities. */
    private String escapeTags(String text) {
        if (text == null) {
            return null;
        }
        return text.replaceAll("<", "&lt;").replaceAll(">", "&gt;");
    }
}

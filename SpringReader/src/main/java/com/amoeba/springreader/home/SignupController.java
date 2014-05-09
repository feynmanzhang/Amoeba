package com.amoeba.springreader.home;

import javax.validation.Valid;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.validation.Errors;
import org.springframework.web.bind.annotation.ModelAttribute;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestMethod;
import org.springframework.web.bind.annotation.RequestParam;
import org.springframework.web.bind.annotation.ResponseBody;
import org.springframework.web.servlet.mvc.support.RedirectAttributes;

import com.amoeba.springreader.domain.UserAccount;
import com.amoeba.springreader.service.UserService;
import com.amoeba.springreader.support.web.MessageHelper;

@Controller
public class SignupController {

//    private static final String SIGNUP_VIEW_NAME = "/signup";
//
//	
//	@Autowired
//	private UserKeywordsService userKeywordsService;
	
	@Autowired
	private UserService userService;
	
//	@RequestMapping(value = "/signup")
//	public String signup(Model model) {
//		model.addAttribute(new SignupForm());
//        return "/index";
//	}
	
	@RequestMapping(value = "signup", method = RequestMethod.POST)
	public String signup(@Valid @ModelAttribute SignupForm signupForm, Errors errors, RedirectAttributes ra) {
		if (errors.hasErrors()) {
			return "/index";
		}

		//	userKeywordsService.create("苹果,众筹,云计算", signupForm.getUsername());// --Default Configuration.	
		UserAccount account = userService.save(signupForm.createAccount());	
		userService.signin(account);
		
        MessageHelper.addSuccessAttribute(ra, "注册成功！");
		return "redirect:/subscribe";
	}
	
	@RequestMapping(value = "validate", method = RequestMethod.GET, produces = "application/json; charset=utf-8")
	public @ResponseBody String validate(@RequestParam String fieldId,@RequestParam String fieldValue, Model model){
		if(fieldId.equals("email")){
			if(userService.isExit(fieldValue))
				return "[\"email\", false]";
		}
		
		return "[\"email\", true]";
	}
	
}

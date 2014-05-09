package com.amoeba.springreader.home;

import java.security.Principal;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.PageRequest;
import org.springframework.data.domain.Pageable;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.PathVariable;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;

import com.amoeba.springreader.domain.PaginationInfo;
import com.amoeba.springreader.domain.UserArticle;
import com.amoeba.springreader.service.UserArticlesService;

@Controller
@RequestMapping("/list")
public class UserArticlesController {

	@Autowired
	UserArticlesService userArticlesService;


	@RequestMapping
	public String list(@RequestParam(required = false) String page,
			@RequestParam(required = false) String pageSize, Model model, Principal principal) {

		int pageLimit = pageSize != null ? Integer.parseInt(pageSize) : 20;
		int currentPage = page != null ? Integer.parseInt(page) : 1;
		String listAction = "/list";

		Pageable pageable = new PageRequest(currentPage - 1, pageLimit);
		Page<UserArticle> articles = userArticlesService.findAll(pageable,principal);
		PaginationInfo pageInfo = new PaginationInfo(currentPage,
				articles.getTotalElements(), pageLimit, listAction);

		model.addAttribute("articles", articles.getContent());
		model.addAttribute("pageInfo", pageInfo);

		return listAction;
	}

	@RequestMapping(value = "/view/{id}")
	public String view(@PathVariable String id, Model model, Principal principal)
			throws Exception {

		//Article article = userArticlesService.findById(id, principal);
	
		//to do update the article had read.

		return "";
	}

}
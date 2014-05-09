package com.amoeba.springreader.home;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.Pageable;
import org.springframework.data.web.PageableDefault;
import org.springframework.stereotype.Controller;
import org.springframework.ui.Model;
import org.springframework.web.bind.annotation.RequestMapping;
import org.springframework.web.bind.annotation.RequestParam;

import com.amoeba.springreader.domain.Article;
import com.amoeba.springreader.domain.PageWrapper;
import com.amoeba.springreader.service.SearchService;

@Controller
@RequestMapping("/search")
public class SearchController {

	@Autowired
	SearchService queryService;

	@RequestMapping
	public String query(@RequestParam String keyword, 
			@PageableDefault(size=20,page=0)Pageable pageable, Model model)  {

    	try {		    		
    		if (keyword == null) return "";	    		    	
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
		
	//	int pageLimit = pagesize != null ? Integer.parseInt(pagesize) : 10;
	//	int currentPage = page != null ? Integer.parseInt(page) : 1;
		String listAction = "/search?keyword=" + keyword;

	//	Pageable pageable = new PageRequest(currentPage - 1, pageLimit);
		Page<Article> articles = queryService.findAll(keyword , pageable);
//		PaginationInfo pageInfo = new PaginationInfo(currentPage,
//				articles.getTotalElements(), pageLimit, listAction);
		
		PageWrapper<Article> pageinfo = new PageWrapper<Article>(articles, listAction);

//		model.addAttribute("articles", articles.getContent());
		model.addAttribute("page", pageinfo);
		model.addAttribute("keyword",keyword);

		return "/fragments/searchresult";
	}
	
	

}
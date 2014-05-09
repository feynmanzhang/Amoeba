package com.amoeba.springreader.service;

import java.security.Principal;
import java.util.List;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.data.domain.Page;
import org.springframework.data.domain.PageImpl;
import org.springframework.data.domain.Pageable;
import org.springframework.stereotype.Service;

import com.amoeba.springreader.UserArticleStatus;
import com.amoeba.springreader.domain.UserArticle;
import com.amoeba.springreader.repository.UserArticlesRepository;

@Service
public class UserArticlesService {

	@Autowired
	private UserArticlesRepository userArticlesRepo;
	
	public boolean create(String articleuuid,String keyword, Principal principal) {
		
		if(userArticlesRepo.findByUsernameAndKeyword(principal.getName(), keyword) != null)
			return false;
		
		userArticlesRepo.save(new UserArticle(articleuuid,principal.getName(),null,
				UserArticleStatus.STATUS_NOREAD.toString(),keyword));
		return true;
	}
	
	public boolean delete(String keywords, Principal principal) {
		return true;
	}
	
	public boolean update(String keywords, Principal principal) {
		return true;
	}

	public List<UserArticle> findByUsernameAndKeyword(String name,String keyword) {
		return userArticlesRepo.findByUsernameAndKeyword(name,keyword);
	}

	public List<UserArticle> findByUsername(String name) {
		return userArticlesRepo.findByUsername(name);
	}
	
	
	
	public Page<UserArticle> findAll(Pageable pageable,Principal principal) {
		List<UserArticle> list = findByUsername(principal.getName());
		
		Page<UserArticle> uiBeans = new PageImpl<UserArticle>(list, pageable,list.size());

		return uiBeans;
	}
}
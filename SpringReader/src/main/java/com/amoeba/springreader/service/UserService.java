package com.amoeba.springreader.service;

import java.util.Collections;
import java.util.List;

import org.springframework.beans.factory.annotation.Autowired;
import org.springframework.security.authentication.UsernamePasswordAuthenticationToken;
import org.springframework.security.core.Authentication;
import org.springframework.security.core.GrantedAuthority;
import org.springframework.security.core.authority.SimpleGrantedAuthority;
import org.springframework.security.core.context.SecurityContextHolder;
import org.springframework.security.core.userdetails.User;
import org.springframework.security.core.userdetails.UserDetails;
import org.springframework.security.core.userdetails.UserDetailsService;
import org.springframework.security.core.userdetails.UsernameNotFoundException;
import org.springframework.stereotype.Service;

import com.amoeba.springreader.UserAccountStatus;
import com.amoeba.springreader.domain.Role;
import com.amoeba.springreader.domain.UserAccount;
import com.amoeba.springreader.repository.RoleRepository;
import com.amoeba.springreader.repository.UserAccountRepository;

@Service
public class UserService implements UserDetailsService{

	@Autowired private UserAccountRepository userAccountRepository;
	
	@Autowired private RoleRepository roleRepository;
	
	public Role getRole(String role) {
		return roleRepository.findOne(role);
	}
	
	public boolean create(UserAccount user) {
		// duplicate username
		if (userAccountRepository.findByUsername(user.getUsername()) != null) {
			return false;
		}
		user.setEnabled(false);
		user.setStatus(UserAccountStatus.STATUS_DISABLED.name());
		userAccountRepository.save(user);
		return true;
	}
	
	public boolean isExit(String username) {
		if (userAccountRepository.findByUsername(username) != null) {
			return true;
		}
		
		return false;
	}
	
	public List<UserAccount> findall() {
		return userAccountRepository.findAll();
	}
	
	public UserAccount save(UserAccount user) {
//		Assert.notNull(user.getId());
		if (userAccountRepository.findByUsername(user.getUsername()) != null) {
			return null;
		}
		return userAccountRepository.save(user);
	}
	
	public void delete(UserAccount user) {
//		Assert.notNull(user.getId());
		userAccountRepository.delete(user);
	}
	
	public void updatePushemail(String username,String pushemail){
		UserAccount userAccount = findByUsername(username);
		userAccountRepository.delete(userAccount);
		userAccount.setPushemail(pushemail);
		userAccountRepository.save(userAccount);
	}
	
	public UserAccount findByUsername(String username) {
		return userAccountRepository.findByUsername(username);
	}
	

	
	@Override
	public UserDetails loadUserByUsername(String username) throws UsernameNotFoundException {
		UserAccount account = userAccountRepository.findByUsername(username);
		if(account == null) {
			throw new UsernameNotFoundException("user not found");
		}
		return createUser(account);
	}
	
	public void signin(UserAccount account) {
		SecurityContextHolder.getContext().setAuthentication(authenticate(account));
	}
	
	private Authentication authenticate(UserAccount account) {
		return new UsernamePasswordAuthenticationToken(createUser(account), null, Collections.singleton(createAuthority(account)));		
	}
	
	private User createUser(UserAccount account) {
		return new User(account.getUsername(), account.getPassword(), Collections.singleton(createAuthority(account)));
	}

	private GrantedAuthority createAuthority(UserAccount account) {
		return new SimpleGrantedAuthority(account.getRoles().toString());
	}

}

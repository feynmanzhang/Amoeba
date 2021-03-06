package com.amoeba.springreader.domain;

import java.util.ArrayList;
import java.util.Iterator;
import java.util.List;

import org.apache.commons.lang3.builder.EqualsBuilder;
import org.apache.commons.lang3.builder.HashCodeBuilder;
import org.springframework.data.annotation.Id;
import org.springframework.data.mongodb.core.index.IndexDirection;
import org.springframework.data.mongodb.core.index.Indexed;
import org.springframework.data.mongodb.core.mapping.DBRef;
import org.springframework.data.mongodb.core.mapping.Document;

@Document
public class UserAccount {
	
	@Id
	private String id;

	@Indexed(unique=true, direction=IndexDirection.DESCENDING, dropDups=true)
	private String username;
	
	private String password;
	private String email;
	private String status;
	private Boolean enabled;
	private String pushemail;
	
//	Map<String, UserKeyword> keywords;
	
	@DBRef
	private List<Role> roles = new ArrayList<Role>();
	
	public  UserAccount() {		
	}
	
	public UserAccount(String username,String email, String password, Role role, String status, Boolean enabled) {
		this.username = username;
		this.email = email;
		this.password = password;
		this.roles.add(role);
		this.status=status;
		this.enabled=enabled;
	}

	public String getId() {
		return id;
	}

	public void setId(String id) {
		this.id = id;
	}

	public String getUsername() {
		return username;
	}

	public void setUsername(String username) {
		this.username = username;
	}

	public String getPassword() {
		return password;
	}

	public void setPassword(String password) {
		this.password = password;
	}
	
	public String getEmail() {
		return email;
	}

	public void setEmail(String email) {
		this.email = email;
	}

	public String getStatus() {
		return status;
	}

	public void setStatus(String status) {
		this.status = status;
	}

	public Boolean getEnabled() {
		return enabled;
	}

	public void setEnabled(Boolean enabled) {
		this.enabled = enabled;
	}
	
	
	public String getPushemail() {
		return pushemail;
	}

	public void setPushemail(String pushemail) {
		this.pushemail = pushemail;
	}

	public List<Role> getRoles() {
		return roles;
	}

	public void addRole(Role role) {
		this.roles.add(role);
	}
	
	public void removeRole(Role role) {
		//use iterator to avoid java.util.ConcurrentModificationException with foreach
		for (Iterator<Role> iter = this.roles.iterator(); iter.hasNext(); )
		{
		   if (iter.next().equals(role))
		      iter.remove();
		}
	}
	
	public String getRolesCSV() {
		StringBuilder sb = new StringBuilder();
		for (Iterator<Role> iter = this.roles.iterator(); iter.hasNext(); )
		{
		   sb.append(iter.next().getId());
		   if (iter.hasNext()) {
			   sb.append(',');
		   }
		}
		return sb.toString();
	}	
	
	public boolean equals(Object obj) {
        if (!(obj instanceof UserAccount)) {
            return false;
        }
        if (this == obj) {
            return true;
        }
        UserAccount rhs = (UserAccount) obj;
        return new EqualsBuilder().append(id, rhs.id).isEquals();
    }

	public int hashCode() {
        return new HashCodeBuilder().append(id).append(username).toHashCode();
    }
}

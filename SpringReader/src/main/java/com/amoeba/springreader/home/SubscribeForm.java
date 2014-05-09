package com.amoeba.springreader.home;

import org.hibernate.validator.constraints.Email;
import org.hibernate.validator.constraints.NotBlank;

public class SubscribeForm {

	private static final String NOT_BLANK_MESSAGE = "{notBlank.message}";
	private static final String EMAIL_MESSAGE = "{email.message}";

	@NotBlank(message = SubscribeForm.NOT_BLANK_MESSAGE)
	private String keyword;
	
    @NotBlank(message = SubscribeForm.NOT_BLANK_MESSAGE)
	@Email(message = SubscribeForm.EMAIL_MESSAGE)
	private String email;

	public String getKeyword() {
		return keyword;
	}

	public void setKeyword(String keyword) {
		this.keyword = keyword;
	}

	public String getEmail() {
		return email;
	}

	public void setEmail(String email) {
		this.email = email;
	}

}

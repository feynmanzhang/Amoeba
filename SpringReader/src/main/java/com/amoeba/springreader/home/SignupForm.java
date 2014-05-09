package com.amoeba.springreader.home;

import javax.validation.constraints.AssertTrue;

import org.hibernate.validator.constraints.Email;
import org.hibernate.validator.constraints.NotBlank;
import org.springframework.security.crypto.bcrypt.BCryptPasswordEncoder;
import org.springframework.security.crypto.password.PasswordEncoder;

import com.amoeba.springreader.UserAccountStatus;
import com.amoeba.springreader.domain.Role;
import com.amoeba.springreader.domain.UserAccount;

public class SignupForm {

//	private static final String NOT_BLANK_MESSAGE = "{notBlank.message}";
//	private static final String EMAIL_MESSAGE = "{email.message}";
	private static final String PASSWORD_NOT_BLANK_MESSAGE = "密码不能为空";
	private static final String EMAIL_NOT_BLANK_MESSAGE = "邮箱不能为空";
	private static final String VERIFY_PASSWORD_MESSAGE = "两次输入的密码不一致，请重新输入";
	private static final String EMAIL_MESSAGE = "邮箱格式无效，请输入正确的邮箱地址";

//	@NotBlank(message = SignupForm.NOT_BLANK_MESSAGE)
//	private String username;
	
    @NotBlank(message = SignupForm.EMAIL_NOT_BLANK_MESSAGE)
	@Email(message = SignupForm.EMAIL_MESSAGE)
	private String email;

    @NotBlank(message = SignupForm.PASSWORD_NOT_BLANK_MESSAGE)
	private String password;
    
    @NotBlank(message = SignupForm.PASSWORD_NOT_BLANK_MESSAGE)
	private String verifypassword;
    
    @AssertTrue(message=SignupForm.PASSWORD_NOT_BLANK_MESSAGE)
    private boolean isValid() {
      return this.password.equals(this.verifypassword);
    }
    
    
//    public String getUsername() {
//		return username;
//	}
//
//	public void setUsername(String username) {
//		this.username = username;
//	}

	public String getEmail() {
		return email;
	}

	public void setEmail(String email) {
		this.email = email;
	}

	public String getPassword() {
		return password;
	}

	public void setPassword(String password) {
		this.password = password;
	}

	public String getVerifypassword() {
		return verifypassword;
	}

	public void setVerifypassword(String verifypassword) {
		this.verifypassword = verifypassword;
	}

	public UserAccount createAccount() {
		PasswordEncoder encoder = new BCryptPasswordEncoder();
		String demoPasswordEncoded = encoder.encode(getPassword());
	//	return new UserAccount(getUsername(),getEmail(), demoPasswordEncoded, new Role("ROLE_USER"),UserAccountStatus.STATUS_APPROVED.name(),true);
		return new UserAccount(getEmail(),getEmail(), demoPasswordEncoded, new Role("ROLE_USER"),UserAccountStatus.STATUS_APPROVED.name(),true);
	}
}

package com.amoeba.Deprecated;

import java.util.Date;
import java.util.Properties;

import javax.mail.Message;
import javax.mail.PasswordAuthentication;
import javax.mail.Session;
import javax.mail.Transport;
import javax.mail.internet.InternetAddress;
import javax.mail.internet.MimeMessage;

import com.amoeba.ScheduledTasks.SendEmail;

public class Test extends Object{

public static void main(String [] args)
{

//	try {
//		SendEmail.Send( "15959013445@139.com", "test", "testformail");
//	} catch (Exception e) {
//		// TODO: handle exception
//	}
	
    try{

        Properties props = new Properties();
        props.put("mail.smtp.host", "smtp.gmail.com"); // for gmail use smtp.gmail.com   smtp.mail.yahoo.com
        props.put("mail.smtp.auth", "true");
//        props.put("mail.debug", "true"); 
        props.put("mail.smtp.starttls.enable", "true");
        props.put("mail.smtp.port", "465");
        props.put("mail.smtp.socketFactory.port", "465");
        props.put("mail.smtp.socketFactory.class", "javax.net.ssl.SSLSocketFactory");
//        props.put("mail.smtp.socketFactory.fallback", "false");

        Session mailSession = Session.getInstance(props,new javax.mail.Authenticator() {

            protected PasswordAuthentication getPasswordAuthentication() {
                return new PasswordAuthentication("feynman0127", "789789zll");
            }
        });

        mailSession.setDebug(true); // Enable the debug mode

        Message msg = new MimeMessage( mailSession );

        //--[ Set the FROM, TO, DATE and SUBJECT fields
        msg.setFrom( new InternetAddress( "feynman0127@gmail.com" ) );
        msg.setRecipients( Message.RecipientType.TO,InternetAddress.parse("15959013445@139.com") );
        msg.setSentDate( new Date());
        msg.setSubject( "Hello World!" );

        //--[ Create the body of the mail
        //msg.setText( "<html><body><p>test</p></body></html>" );
        msg.setContent("<html> <body><h1>Hello </h1> </body></html>", "text/html");

        //--[ Ask the Transport class to send our mail message
        Transport.send( msg );

    }catch(Exception E){
        System.out.println( "Oops something has gone pearshaped!");
        System.out.println( E );
    }
}
}
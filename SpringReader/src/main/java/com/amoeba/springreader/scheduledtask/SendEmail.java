package com.amoeba.springreader.scheduledtask;

import java.util.Properties;

import javax.mail.Message;
import javax.mail.MessagingException;
import javax.mail.PasswordAuthentication;
import javax.mail.Session;
import javax.mail.Transport;
import javax.mail.internet.AddressException;
import javax.mail.internet.InternetAddress;
import javax.mail.internet.MimeMessage;

public class SendEmail {

    public static void Send( String recipientEmail, String title, String message) throws AddressException, MessagingException {
    	SendEmail.Send("", "", recipientEmail, "", title, message);
    }
    
    /**
     * Send email.
     *
     * @param username
     * @param password
     * @param recipientEmail TO recipient
     * @param ccEmail CC recipient. Can be empty if there is no CC recipient
     * @param title title of the message
     * @param message message to be sent
     * @throws AddressException if the email address parse failed
     * @throws MessagingException if the connection is dead or not in the connected state or if the message is not a MimeMessage
     */
    public static void Send(final String username, final String password, String recipientEmail, String ccEmail, String title, String message) throws AddressException, MessagingException {

        Properties props = new Properties();
        props.put("mail.smtp.host", "smtp.gmail.com");
        props.put("mail.smtp.starttls.enable", "true");
        props.setProperty("mail.smtp.socketFactory.class", "javax.net.ssl.SSLSocketFactory");
//        props.setProperty("mail.smtp.socketFactory.fallback", "false");
        props.setProperty("mail.smtp.socketFactory.port", "465");
        props.put("mail.smtp.port", "465");
        props.put("mail.smtp.auth", "true");
        props.put("mail.debug", false); 


        Session session = Session.getInstance(props,new javax.mail.Authenticator() {

            protected PasswordAuthentication getPasswordAuthentication() {
                return new PasswordAuthentication("feynman.zhang", "amoeba789789");
            }
        });


        // -- Create a new message --
        final MimeMessage msg = new MimeMessage(session);
        msg.setFrom( new InternetAddress( "feynman.zhang@gmail.com" ) );
        msg.setRecipients(Message.RecipientType.TO, InternetAddress.parse(recipientEmail, false));

        if (ccEmail.length() > 0) {
            msg.setRecipients(Message.RecipientType.CC, InternetAddress.parse(ccEmail, false));
        }

        msg.setSubject(title);
        msg.setText(message, "utf-8","html");
        Transport.send( msg );
    }
    
    
    public static void main(String [] args)
    {

    	try {
    		SendEmail.Send( "15959013445@139.com", "test", "<html> <body><h1>中文艹1 </h1> </body></html>");
    	} catch (Exception e) {
    		// TODO: handle exception
    	}
    }
}

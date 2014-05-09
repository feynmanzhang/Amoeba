package com.amoeba.Deprecated;

import org.apache.commons.logging.Log;
import org.apache.commons.logging.LogFactory;
public class HelloLog4j {
//private static Logger logger = Logger.getLogger(HelloLog4j.class);

public static Log logger = LogFactory.getLog(HelloLog4j.class);

	public HelloLog4j(){
		//PropertyConfigurator.configure("log4j.properties");
	}

public static void main(String[] args) {
	
	logger.info(System.getProperty("user.dir"));
	//  记录debug 级别的信息
	logger.debug("This is debug message.");
	//  记录info 级别的信息
	logger.info("This is info message.");
	//  记录error 级别的信息
	logger.error("This is error message.");
	}
}
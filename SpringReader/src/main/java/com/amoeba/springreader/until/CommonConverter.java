package com.amoeba.springreader.until;

import java.text.SimpleDateFormat;
import java.util.Date;
import java.util.TimeZone;

public class CommonConverter {

	public static String encodeConverter(String str){
		try {		    		
			if (str == null) return "";	    		    	
	    	if (str.equals(new String(str.getBytes("ISO8859-1"),"ISO8859-1"))) {
	    		str=new String(str.getBytes("ISO8859-1"),"UTF-8");  
			}
	    	else if(str.equals(new String(str.getBytes("UTF-8"),"UTF-8"))){
	    		str=new String(str.getBytes("UTF-8"),"UTF-8");  
	    	}
	    	else if(str.equals(new String(str.getBytes("GBK"),"GBK"))){
	    		str=new String(str.getBytes("GBK"),"UTF-8");  
	    	}	    	
		} catch (Exception e) {
			// TODO: handle exceptionXx
			e.printStackTrace();
		}
		
		return str;
	}
	
	public static String toDateStringFromIso(String sdate) { 
        if ("null".equals(sdate) || "NULL".equals(sdate) || "".equals(sdate) || sdate == null) {
            return "";
        }
      
//        Date date = new Date(sdate);
//        return date.getMonth() + "/" + date.getDate();
        
        SimpleDateFormat dateFormat1 = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss.SSS");
        dateFormat1.setLenient(false);
        dateFormat1.setTimeZone(TimeZone.getTimeZone("UTC")); 
        SimpleDateFormat dateFormat2 = new SimpleDateFormat("yy/MM/dd");
        Date nowDate;
        try {
            nowDate = dateFormat1.parse(sdate);             
		} catch (Exception e) {
			return "";
		}
        
        String dateString = dateFormat2.format(nowDate); 
        return dateString;
    }
	
	public static String delTagsFContent(String content){  
        
//        String strClear=strHtml.replaceAll( ".*?<body.*?>(.*?)<\\/body>", "$1"); //读出body内里所有内容  
//        strClear=strClear.replaceAll("</?[^/?(br)|(p)][^><]*>","");//保留br标签和p标签  
        return content.replaceAll("</?[a-zA-Z]+[^><]*>", "");//</?.*/?>
	}  
	
	public static void main(String [] args){
		
		System.out.print(toDateStringFromIso("2014-03-12T14:22:24.594Z"));
		
		System.out.print(delTagsFContent("<img 12341>1341</br><br>"));
	}

}

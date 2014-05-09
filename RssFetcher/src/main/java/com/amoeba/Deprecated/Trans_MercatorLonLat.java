package com.amoeba.Deprecated;

/**
 * 墨卡托和经纬度坐标互转
 * @author zll
 * 
 */

public class Trans_MercatorLonLat {
	public static double[] lonLat2Mercator(double lon,double lat)
	{
		double[] ptMercator =  new double[2];
		ptMercator[0] = lon*20037508.342787001/180;
		ptMercator[1] = (Math.log(Math.tan((90+lat)*Math.PI/360))/(Math.PI/180))* 20037508.342787001/180;
		return ptMercator;
	}
	
	public static double[] mercator2LonLat(double mercatorX,double mercatorY)
	{
		 double[] ptLonLat = new double[2];
		 ptLonLat[0] = mercatorX/20037508.342787001*180;
		 ptLonLat[1] = 180/Math.PI*(2*Math.atan(Math.exp(mercatorY/20037508.342787001*180*Math.PI/180))-Math.PI/2);
	     return ptLonLat;
	}
	
	public static void main(String[] args) {
		double[]pt = lonLat2Mercator(119.33,26.07);
		System.out.println(pt[0]+","+ pt[1]);
		double[]pt2 = mercator2LonLat(1.356766385406494E7,2605885.3079833984
);
		System.out.println(pt2[0]+","+ pt2[1]);
		
		double[]pt3 = mercator2LonLat(13280800.651568817, 3008947.9493408003);
		System.out.println(pt3[0]+","+ pt3[1]);
	}
}


    $(document).ready(function(){
    	$("#keywordSearch").click(
	    	function(){
	    		if($("#keyword").val() == ""){
	    			alert("keyword is empty!");
	    		} else {
		    		$.get("/SpringReader/search?keyword=" + $("#keyword").val(),function(data,status){  			
		    			$("#searchResult").html(data);
		    		});
	    		}  		
	    	}
    	);
    	$("#keywordPending").click(
	    	function(){
	    		if($("#keyword").val() == ""){
	    			alert("keyword is empty!");
	    		} else {
		    		$("#tagsinput").tagsinput('add', $("#keyword").val());
	    		}  		
	    	}
        );
    	
    	$("#keywordSave").click(
	    	function(){
	    		$.get("/SpringReader/keyword/post?keywords=" + $("#tagsinput").val(),function(data,status){  			
	    			alert(status);
	    		}); 		
	    	}
        );
    	
    	$("#keywordSetting").click(
    		function(){
    			window.location.href='/SpringReader/setting';	
    		}    		
    	);
    	
    	$("#settingSave").click(
	    	function(){
	    		$.get("/SpringReader/setting/post?pushemail=" + $("#pushemail").val(),function(data,status){  			
	    			alert(status);
	    		}); 		
	    	}
	    );
    });
    
    
	function paginationSearch(url){
		$.get(url,function(data,status){  			
			$("#searchResult").html(data);
		});
	}
	
	
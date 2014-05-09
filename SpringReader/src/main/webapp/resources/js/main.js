    $(document).ready(function(){	

    	$('.pure-form').validationEngine({
    		focusFirstField : false,
    		maxErrorsPerField : 1,
  //  		showOneMessage: true,
    		showPrompts : true,
    		promptPosition : "bottomLeft",
    		scroll : false
    	});  
    	
    	$("#keyword").keyup(
    		function(){
    	    	if($("#keyword").val() =="")
    	    		return;
	    		$.get(encodeURI("search?keyword=" + $("#keyword").val()),function(data,status){  			
	    			$("#searchResult").html(data);
	    		});
    		}	
    	);
    	
    	// this is the id of the form
    	$("#createsubscribeform").submit(function() {

    		if( !$("#createsubscribeform").validationEngine('validate')){
    			return false;
    		}
    		
    	    var url = "createsubscribe"; // the script where you handle the form input.

    	    $.ajax({
    	           type: "get",
    	           url: url,
    	           data: $("#createsubscribeform").serialize(), // serializes the form's elements.
    	           dataType: "json",
    	           success: function(result)
    	           { 	
    	        	   if(result.success)
    	        		   location.hash="modal-text";
    	        	   else
    	        		   alert("订阅失败：" + result.message); 
    	           },
    	    	   error: function(XMLHttpRequest, textStatus, errorThrown)
    	           {
    	               alert("订阅失败："  + textStatus +errorThrown); 
    	           }
    	         });

    	    return false; // avoid to execute the actual submit of the form.
    	});
//    	$("#createsubscribe").click(
//    		function(){
//    	    	if($("#keyword").val() =="" || $("#email").val() =="")
//    	    		return;
////	    		$.get(encodeURI("/SpringReader/createsubscribe?keyword=" + $("#keyword").val() + "&email=" + $("#email").val()),function(data){ 
////	    			alert("");
////	    			alert(data);
////	    			window.location.href=data;
////	    		},"html"); 
//	    		
//	    		$.ajax({
//	    		    url:encodeURI("createsubscribe?keyword=" + $("#keyword").val() + "&email=" + $("#email").val()),
//	    		    dataType: 'json',
//	    		    type:"get",
//	    		    cache: false,
//	    		    
//	    		    beforeSend: function () {
//	    		        console.log("Loading");
//	    		    },
//
//	    		    error: function (jqXHR, textStatus, errorThrown) {
//	    		    	window.location.href="manage";
//	    		    },
//
//	    		    success: function (data) {
//	    		    	window.location.href="manage";
//	    		    },
//
//	    		    complete: function () {
//	    		        console.log('Finished all tasks');
//	    		    }
//	    		});
//    		}
//    	);
        $("#deletesubscribe").click(function() {    
            // 判断是否至少选择一项    
            var checkedNum = $("input[name='subcheck']:checked").length;    
            if(checkedNum == 0) {    
                alert("请选择至少一项！");    
                return;    
            }    
                
            // 批量选择     
            if(confirm("确定要删除所选项目？")) {    
                var checkedList = new Array();    
                $("input[name='subcheck']:checked").each(function() {    
                    checkedList.push($(this).val());    
                });    
        
                $.ajax({    
                    type: "POST",    
                    url: "deletesubscribe",    
                    data: "ids=" + checkedList.toString(),  
                    success: function(result) {    
                        window.location.reload();    
                    }    
                });    
            }               
        }); 
        
        $('input, textarea').placeholder();
    });
    
    
	function paginationSearch(url){
		$.get(encodeURI(url),function(data,status){  			
			$("#searchResult").html(data);
		});
	}
	
	
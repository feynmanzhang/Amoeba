(function($){
    $.fn.validationEngineLanguage = function(){
    };
    $.validationEngineLanguage = {
        newLang: function(){
            $.validationEngineLanguage.allRules = {
                "required": { // Add your regex rules here, you can take telephone as an example
                    "regex": "none",
                    "alertText": "该项不可为空",
                    "alertTextCheckboxMultiple": "请选择一个项目",
                    "alertTextCheckboxe": "您必须钩选此栏",
                    "alertTextDateRange": "日期范围不可为空"
                },
                "requiredInFunction": { 
                    "func": function(field, rules, i, options){
                        return (field.val() == "test") ? true : false;
                    },
                    "alertText": "Field must equal test"
                },
                "dateRange": {
                    "regex": "none",
                    "alertText": "无效的 ",
                    "alertText2": " 日期范围"
                },
                "dateTimeRange": {
                    "regex": "none",
                    "alertText": "无效的 ",
                    "alertText2": " 时间范围"
                },
                "minSize": {
                    "regex": "none",
                    "alertText": "至少 ",
                    "alertText2": " 个字符"
                },
                "maxSize": {
                    "regex": "none",
                    "alertText": "最多 ",
                    "alertText2": " 个字符"
                },
				"groupRequired": {
                    "regex": "none",
                    "alertText": "你必须选填其中一个栏位"
                },
                "min": {
                    "regex": "none",
                    "alertText": "最小值为 "
                },
                "max": {
                    "regex": "none",
                    "alertText": "最大值为 "
                },
                "past": {
                    "regex": "none",
                    "alertText": "日期必须早于 "
                },
                "future": {
                    "regex": "none",
                    "alertText": "日期必须晚于 "
                },	
                "maxCheckbox": {
                    "regex": "none",
                    "alertText": "最多选取 ",
                    "alertText2": " 个项目"
                },
                "minCheckbox": {
                    "regex": "none",
                    "alertText": "请选择 ",
                    "alertText2": " 个项目"
                },
                "equals": {
                    "regex": "none",
                    "alertText": "两次输入的密码不一致"
                },
                "creditCard": {
                    "regex": "none",
                    "alertText": "无效的信用卡号码"
                },
                "phone": {
                    // credit: jquery.h5validate.js / orefalo
                    "regex": /^([\+][0-9]{1,3}[ \.\-])?([\(]{1}[0-9]{2,6}[\)])?([0-9 \.\-\/]{3,20})((x|ext|extension)[ ]?[0-9]{1,4})?$/,
                    "alertText": "无效的电话号码"
                },
                "email": {
                    // Shamelessly lifted from Scott Gonzalez via the Bassistance Validation plugin http://projects.scottsplayground.com/email_address_validation/
                    "regex": /^((([a-z]|\d|[!#\$%&'\*\+\-\/=\?\^_`{\|}~]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])+(\.([a-z]|\d|[!#\$%&'\*\+\-\/=\?\^_`{\|}~]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])+)*)|((\x22)((((\x20|\x09)*(\x0d\x0a))?(\x20|\x09)+)?(([\x01-\x08\x0b\x0c\x0e-\x1f\x7f]|\x21|[\x23-\x5b]|[\x5d-\x7e]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])|(\\([\x01-\x09\x0b\x0c\x0d-\x7f]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF]))))*(((\x20|\x09)*(\x0d\x0a))?(\x20|\x09)+)?(\x22)))@((([a-z]|\d|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])|(([a-z]|\d|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])([a-z]|\d|-|\.|_|~|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])*([a-z]|\d|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])))\.)+(([a-z]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])|(([a-z]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])([a-z]|\d|-|\.|_|~|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])*([a-z]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])))\.?$/i,
                    "alertText": "邮箱地址无效"
                },
                "integer": {
                    "regex": /^[\-\+]?\d+$/,
                    "alertText": "不是有效的整数"
                },
                "number": {
                    // Number, including positive, negative, and floating decimal. credit: orefalo
                    "regex": /^[\-\+]?((([0-9]{1,3})([,][0-9]{3})*)|([0-9]+))?([\.]([0-9]+))?$/,
                    "alertText": "无效的数字"
                },
                "date": {
                    "regex": /^\d{4}[\/\-](0?[1-9]|1[012])[\/\-](0?[1-9]|[12][0-9]|3[01])$/,
                    "alertText": "无效的日期，格式必须为 YYYY-MM-DD"
                },
                "ipv4": {
                    "regex": /^((([01]?[0-9]{1,2})|(2[0-4][0-9])|(25[0-5]))[.]){3}(([0-1]?[0-9]{1,2})|(2[0-4][0-9])|(25[0-5]))$/,
                    "alertText": "无效的 IP 地址"
                },
                "url": {
                    "regex": /^(https?|ftp):\/\/(((([a-z]|\d|-|\.|_|~|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])|(%[\da-f]{2})|[!\$&'\(\)\*\+,;=]|:)*@)?(((\d|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])\.(\d|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])\.(\d|[1-9]\d|1\d\d|2[0-4]\d|25[0-5])\.(\d|[1-9]\d|1\d\d|2[0-4]\d|25[0-5]))|((([a-z]|\d|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])|(([a-z]|\d|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])([a-z]|\d|-|\.|_|~|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])*([a-z]|\d|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])))\.)+(([a-z]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])|(([a-z]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])([a-z]|\d|-|\.|_|~|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])*([a-z]|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])))\.?)(:\d*)?)(\/((([a-z]|\d|-|\.|_|~|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])|(%[\da-f]{2})|[!\$&'\(\)\*\+,;=]|:|@)+(\/(([a-z]|\d|-|\.|_|~|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])|(%[\da-f]{2})|[!\$&'\(\)\*\+,;=]|:|@)*)*)?)?(\?((([a-z]|\d|-|\.|_|~|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])|(%[\da-f]{2})|[!\$&'\(\)\*\+,;=]|:|@)|[\uE000-\uF8FF]|\/|\?)*)?(\#((([a-z]|\d|-|\.|_|~|[\u00A0-\uD7FF\uF900-\uFDCF\uFDF0-\uFFEF])|(%[\da-f]{2})|[!\$&'\(\)\*\+,;=]|:|@)|\/|\?)*)?$/i,
                    "alertText": "无效的 URL 地址"
                },
                "onlyNumberSp": {
                    "regex": /^[0-9\ ]+$/,
                    "alertText": "只能填数字"
                },
                "onlyLetterSp": {
                    "regex": /^[a-zA-Z\ \']+$/,
                    "alertText": "只接受英文字母大小写"
                },
                "onlyLetterNumber": {
                    "regex": /^[0-9a-zA-Z]+$/,
                    "alertText": "不接受特殊字符"
                },
                // --- CUSTOM RULES -- Those are specific to the demos, they can be removed or changed to your likings
                "telephone": {
                	"regex": /^[0-9\-\|]*$/,
                	"alertText": "只接受数字，'-'和'|'"
                },
                "wildcard": {
                	"regex": /^[0-9a-zA-Z_\-\:\*]*$/,
                    "alertText": "只接受英文字母，数字，'_'，'-'，':'和'*'"
                },
                "noSpecChar": {
                	"regex": /^[a-zA-Z0-9_\-\u4E00-\u9FA5]*$/,
                	"alertText": "只接受中文，英文字母，数字，下划线和'-'"		
                },
                "username": {
                	"regex": /^[0-9a-zA-Z_]*$/,
                	"alertText": "只接受英文字母，数字，下划线"
                },
                "mobilePhone": {
                	"regex": /^1[3|4|5|8][0-9]\d{4,8}$/,
                	"alertText": "无效的手机号码"
                },
                "ajaxUserCall": {
                    "url": "ajaxValidateFieldUser",
                    // you may want to pass extra data on the ajax call
                    "extraData": "name=eric",
                    "alertText": "此名称已被其他人使用",
                    "alertTextLoad": "正在确认名称是否有其他人使用，请稍等。"
                },
				"ajaxUserCallPhp": {
                    "url": "phpajax/ajaxValidateFieldUser.php",
                    // you may want to pass extra data on the ajax call
                    "extraData": "name=eric",
                    // if you provide an "alertTextOk", it will show as a green prompt when the field validates
                    "alertTextOk": "此帐号名称可以使用",
                    "alertText": "此名称已被其他人使用",
                    "alertTextLoad": "正在确认帐号名称是否有其他人使用，请稍等。"
                },
                "ajaxNameCall": {
                    // remote json service location
                    "url": "ajaxValidateFieldName",
                    // error
                    "alertText": "此名称可以使用",
                    // if you provide an "alertTextOk", it will show as a green prompt when the field validates
                    "alertTextOk": "此名称已被其他人使用",
                    // speaks by itself
                    "alertTextLoad": "正在确认名称是否有其他人使用，请稍等。"
                },
				"ajaxNameCallPhp": {
	                    // remote json service location
	                    "url": "phpajax/ajaxValidateFieldName.php",
	                    // error
	                    "alertText": "此名称已被其他人使用",
	                    // speaks by itself
	                    "alertTextLoad": "正在确认名称是否有其他人使用，请稍等。"
	                },
                "validate2fields": {
                    "alertText": "请输入 HELLO"
                },
	            //tls warning:homegrown not fielded 
                "dateFormat":{
                    "regex": /^\d{4}[\/\-](0?[1-9]|1[012])[\/\-](0?[1-9]|[12][0-9]|3[01])$|^(?:(?:(?:0?[13578]|1[02])(\/|-)31)|(?:(?:0?[1,3-9]|1[0-2])(\/|-)(?:29|30)))(\/|-)(?:[1-9]\d\d\d|\d[1-9]\d\d|\d\d[1-9]\d|\d\d\d[1-9])$|^(?:(?:0?[1-9]|1[0-2])(\/|-)(?:0?[1-9]|1\d|2[0-8]))(\/|-)(?:[1-9]\d\d\d|\d[1-9]\d\d|\d\d[1-9]\d|\d\d\d[1-9])$|^(0?2(\/|-)29)(\/|-)(?:(?:0[48]00|[13579][26]00|[2468][048]00)|(?:\d\d)?(?:0[48]|[2468][048]|[13579][26]))$/,
                    "alertText": "无效的日期格式"
                },
                //tls warning:homegrown not fielded 
				"dateTimeFormat": {
	                "regex": /^\d{4}[\/\-](0?[1-9]|1[012])[\/\-](0?[1-9]|[12][0-9]|3[01])\s+(1[012]|0?[1-9]){1}:(0?[1-5]|[0-6][0-9]){1}:(0?[0-6]|[0-6][0-9]){1}\s+(am|pm|AM|PM){1}$|^(?:(?:(?:0?[13578]|1[02])(\/|-)31)|(?:(?:0?[1,3-9]|1[0-2])(\/|-)(?:29|30)))(\/|-)(?:[1-9]\d\d\d|\d[1-9]\d\d|\d\d[1-9]\d|\d\d\d[1-9])$|^((1[012]|0?[1-9]){1}\/(0?[1-9]|[12][0-9]|3[01]){1}\/\d{2,4}\s+(1[012]|0?[1-9]){1}:(0?[1-5]|[0-6][0-9]){1}:(0?[0-6]|[0-6][0-9]){1}\s+(am|pm|AM|PM){1})$/,
                    "alertText": "无效的日期或时间格式",
                    "alertText2": "可接受的格式： ",
                    "alertText3": "mm/dd/yyyy hh:mm:ss AM|PM 或 ", 
                    "alertText4": "yyyy-mm-dd hh:mm:ss AM|PM"
	            },"ajaxUser":{
                        "url":"register/vali/",//
	                    "extraData": "name=eric",
	                    "alertTextOk": "* 此帐号名称可以使用",
	                    "alertText": "* 此名称已被其他人使用",
	                  //  "alertTextLoad": "* 正在确认帐号名称是否有其他人使用，请稍等。"
	                    "alertTextLoad": "* 请稍等。"
                    }, 
                  "ajaxPhone": {
                  "url": "register/vali/",
                    "extraData": "name=eric",
                    "alertText": "* 此手机号码已被其他人注册",
                      "alertTextOk": "* 此手机号码可以使用",
                    //"alertTextLoad": "* 正在确认号码是否有其他人注册，请稍等。"
                      "alertTextLoad": "* 请稍等。"
                },
                 "ajaxEmail": {
                  "url": "validate",
                    "extraData": "",
                    "alertText": "* 此Email已被注册",
                      "alertTextOk": "* 此Email可以使用",
                   // "alertTextLoad": "* 正在确认Email是否有其他人注册，请稍等。"
                      "alertTextLoad": "* 请稍等。"
                },
                "ajaxcaptcha":{
                	 "url": "register/vali/",
                    "extraData": "name=eric",
                    "alertText": "* 验证码错误",
                      "alertTextOk": "* 验证码正确",
                    "alertTextLoad": "* 请稍等。"
                },
                "ajaxfindcaptcha":{
                	 "url": "../../register/vali/",
                    "extraData": "name=eric",
                    "alertText": "* 验证码错误",
                      "alertTextOk": "* 验证码正确",
                    "alertTextLoad": "* 请稍等。"
                },
                 "ajaxappname":{
                	 "url": "../../app/vali/",
                    "extraData": "name=eric",  
                    "alertText": "* 此名称已被使用",
                      "alertTextOk": "* 此名称可以使用",
                    "alertTextLoad": "* 请稍等。"
                },
                "ajaxeditappname":{
                	 "url": "../../../app/vali/",
                    "extraData": "name=eric",  
                    "alertText": "* 此名称已被使用",
                      "alertTextOk": "* 此名称可以使用",
                    "alertTextLoad": "* 请稍等。"
                },
                  "ajaxmapname":{
                	 "url": "../../map/vali/",
                    "extraData": "name=mapname",
                    "alertText": "* 此名称已被使用",
                      "alertTextOk": "* 此名称可以使用",
                    "alertTextLoad": "* 请稍等。"
                },
                  
                  "ajaxmapstylename":{
                	 "url": "../../map/vali/",
                    "extraData": "name=stylename",
                    "alertText": "* 此名称已被使用",
                      "alertTextOk": "* 此名称可以使用",
                    "alertTextLoad": "* 请稍等。"
                },
                "ajaxservicesname":{
                	 "url": "../../../services/vali/",
                    "extraData": "name=stylename",
                    "alertText": "* 此名称已被使用",
                      "alertTextOk": "* 此名称可以使用",
                    "alertTextLoad": "* 请稍等。"
                },
                "ajaxuserpwd":{
                	 "url": "../user/register/vali/pwd/",
                    "extraData": "name=stylename",
                    "alertText": "* 与原密码相同，请重新输入！",
                      "alertTextOk": "* 此名称可以使用",
                    "alertTextLoad": "* 请稍等。"
                },"ajaxusercatc":{
                	 "url": "../user/register/vali/pwd/",
                    "extraData": "name=stylename",
                    "alertText": "* 验证码错误",
                      "alertTextOk": "* 此名称可以使用",
                    "alertTextLoad": "* 请稍等。"
                },
                "ajaxeditPhone": {
                  "url": "../register/vali/",
                    "extraData": "name=eric",
                    "alertText": "* 此手机号码已被其他人注册",
                      "alertTextOk": "* 此手机号码可以使用",
                    //"alertTextLoad": "* 正在确认号码是否有其他人注册，请稍等。"
                      "alertTextLoad": "* 请稍等。"
                },
                 "ajaxeditEmail": {
                  "url": "../register/vali/",
                    "extraData": "name=eric",
                    "alertText": "* 此Email已被其他人注册",
                      "alertTextOk": "* 此Email可以使用",
                   // "alertTextLoad": "* 正在确认Email是否有其他人注册，请稍等。"
                      "alertTextLoad": "* 请稍等。"
                },"repwd":{
                	 "regex":/^(?![a-zA-z]+$)(?!\d+$)(?![!@#$%^&*]+$)[a-zA-Z\d!@#$%^&*]+$/,
                    "alertText": "密码强度弱,必须包含数字或字母!"
                }
                
                
            };
            
        }
    };
    $.validationEngineLanguage.newLang();
})(jQuery);

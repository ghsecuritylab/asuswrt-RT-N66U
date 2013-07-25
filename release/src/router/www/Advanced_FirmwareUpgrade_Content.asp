﻿<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_6_3#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<style>
.Bar_container{
	width:85%;
	height:21px;
	border:2px inset #999;
	margin:0 auto;
	background-color:#FFFFFF;
	z-index:100;
}
#proceeding_img_text{
	position:absolute; z-index:101; font-size:11px; color:#000000; margin-left:110px; line-height:21px;
}
#proceeding_img{
 	height:21px;
	background:#C0D1D3 url(/images/proceeding_img.gif);
}
</style>

<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script type="text/javascript" language="JavaScript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/jquery.js"></script>
<script language="JavaScript" type="text/javascript" src="/ajax.js"></script>
<script>
	
var $j = jQuery.noConflict();	
wan_route_x = '<% nvram_get("wan_route_x"); %>';
wan_nat_x = '<% nvram_get("wan_nat_x"); %>';
wan_proto = '<% nvram_get("wan_proto"); %>';

var varload = 0;
var count=0;

function initial(){
	show_menu();	
	if(live_update_support == -1)
		$("update").style.display = "none";
	else
		detect_firmware();
}

var exist_firmver="<% nvram_get("firmver"); %>";
function detect_firmware(){

	$j.ajax({
    		url: '/detect_firmware.asp',
    		dataType: 'script', 
	
    		error: function(xhr){
    				setTimeout("detect_firmware();", 1000);
    		},
    		
    		success: function(){
      			if(webs_state_update==0){
      					count =count+1;
      					//$('update_states').innerHTML="<#check_proceeding#>";
      					$('update_scan').style.display="none";
      					setTimeout("detect_firmware();", 1000);      				
      			}else{
      					if(webs_state_error==1){
      							//if(count>=1){
      								$('update_states').innerHTML="<#connect_failed#>";
      								$('update_scan').style.display="none";
      								return;
      							//}	
      					}else{
      			      				      					
	      					var Latest_firmver = webs_state_info.split("_");
	      					var Latest_firm = Latest_firmver[0];
	      					var Latest_buildno = Latest_firmver[1];
      						current_firm = parseInt(exist_firmver.replace(/[.]/gi,""));
      						current_buildno = <% nvram_get("buildno"); %>;
      						//alert(Latest_firm+' , '+Latest_buildno+' , '+current_firm+' , '+current_buildno);
      						if(current_firm < Latest_firm){
      							  //$('update_states').innerHTML="<#exist_new#>";
      								$('update_scan').style.display="none";
      								if(confirm("<#exist_new#>")){
      										document.start_update.action_mode.value="apply";
      										document.start_update.action_script.value="start_webs_upgrade";
      										document.start_update.action_wait.value="300";
													document.start_update.submit();
													showLoadingBar(450);
													return;
      								}
      						}else if(current_firm == Latest_firm && current_buildno < Latest_buildno){
      							  //$('update_states').innerHTML="<#exist_new#>";
      								$('update_scan').style.display="none";
      								if(confirm("<#exist_new#>")){
      										document.start_update.action_mode.value="apply";
      										document.start_update.action_script.value="start_webs_upgrade";
      										document.start_update.action_wait.value="300";
													document.start_update.submit();
													showLoadingBar(450);
													return;
      								}      							      							
      						}else{
      								var flag = getCookie("after_check");      							
      								if(flag==1){
      							  		$('update_states').innerHTML="<#is_latest#>";
      										$('update_scan').style.display="none";      									
      										setCookie("after_check", 0, 365);
      								}	
      						}	
								}
							}	
  		}
  		});
}

function detect_update(){	
	setCookie("after_check", 1, 365);
  document.start_update.action_mode.value="apply";
  document.start_update.action_script.value="start_webs_update";
  document.start_update.action_wait.value="12";
  $('update_states').innerHTML="<#check_proceeding#>";
  $('update_scan').style.display="";
	document.start_update.submit();
}

function getCookie(c_name)
{
var i,x,y,ARRcookies=document.cookie.split(";");
for (i=0;i<ARRcookies.length;i++)
  {
  x=ARRcookies[i].substr(0,ARRcookies[i].indexOf("="));
  y=ARRcookies[i].substr(ARRcookies[i].indexOf("=")+1);
  x=x.replace(/^\s+|\s+$/g,"");
  if (x==c_name)
    {
    return unescape(y);
    }
  }
}

function setCookie(c_name,value,exdays)
{
var exdate=new Date();
exdate.setDate(exdate.getDate() + exdays);
var c_value=escape(value) + ((exdays==null) ? "" : "; expires="+exdate.toUTCString());
document.cookie=c_name + "=" + c_value;
}

function checkCookie()
{
var aft_chk=getCookie("after_check");
if (aft_chk!=null && aft_chk!="")
  {
  	aft_chk=parseInt(aft_chk)+1;
  	setCookie("after_check", aft_chk, 365);
  }
else 
  {
    setCookie("after_check", 0, 365);
  }
  
return getCookie("after_check");
}
</script>
</head>

<body onload="initial();">

<div id="TopBanner"></div>

<div id="LoadingBar" class="popup_bar_bg">
<table cellpadding="5" cellspacing="0" id="loadingBarBlock" class="loadingBarBlock" align="center">
	<tr>
		<td height="80">
		<div class="Bar_container">
			<span id="proceeding_img_text" ></span>
			<div id="proceeding_img"></div>
		</div>
		<div style="margin:5px auto; width:85%;"><#FIRM_ok_desc#></div>
		</td>
	</tr>
</table>
<!--[if lte IE 6.5]><iframe class="hackiframe"></iframe><![endif]-->
</div>
<div id="Loading" class="popup_bg"></div><!--for uniform show, useless but have exist-->

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" action="upgrade.cgi" name="form" target="hidden_frame" enctype="multipart/form-data">
<input type="hidden" name="current_page" value="Advanced_FirmwareUpgrade_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">

<table class="content" align="center" cellpadding="0" cellspacing="0">
	<tr>
		<td width="17">&nbsp;</td>
		
		<td valign="top" width="202">				
		<div id="mainMenu"></div>	
		<div id="subMenu"></div>		
		</td>				
		
    <td valign="top">
	<div id="tabMenu" class="submenuBlock"></div>
		<!--===================================Beginning of Main Content===========================================-->
<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
	<tr>
		<td align="left" valign="top" >
          
		<table width="760px" border="0" cellpadding="5" cellspacing="0" class="FormTitle" id="FormTitle">
		<tbody>
		<tr>
		  <td bgcolor="#4D595D" valign="top"  >
		  <div>&nbsp;</div>
		  <div class="formfonttitle"><#menu5_6_adv#> - <#menu5_6_3#></div>
		  <div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
		  <div class="formfontdesc"><#FW_desc1#></div>

		<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
			<tr>
				<th><#FW_item1#></th>
				<td><input type="text" class="input_15_table" value="<% nvram_get("productid"); %>" readonly="1"></td>
			</tr>
			<tr>
				<th><#FW_item2#></th>
				<td><input type="text" name="firmver_table" class="input_15_table" value="<% nvram_get("firmver"); %>.<% nvram_get("buildno"); %>" readonly="1"><!--/td-->
						<input type="button" id="update" name="update" class="button_gen" onclick="detect_update();" value="<#liveupdate#>" />
						<div id="check_states">
								<img id="update_scan" style="display:none;" src="images/InternetScan.gif" />
								<span id="update_states"></span>								
						</div>							
				</td>
			</tr>
			<tr>
				<th><#FW_item5#></th>
				<td><input type="file" name="file" class="input" style="color:#FFCC00;"></td>
			</tr>
			<tr align="center">
			  <td colspan="2"><input type="button" name="button" class="button_gen" onclick="onSubmitCtrlOnly(this, 'Upload1');" value="<#CTL_upload#>" /></td>
			</tr>
			<tr>
				<td colspan="2">
				<strong><#FW_note#></strong>
				<ol>
					<li><#FW_n1#></li>
					<li><#FW_n2#></li>
				</ol>
				</td>
			</tr>															
		</table>
			  </td>
              </tr>
            </tbody> 
            </table>
		  </td>


        </tr>
      </table>				
		<!--===================================Ending of Main Content===========================================-->		
	</td>
		
    <td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>

<div id="footer"></div>
</form>

<form method="post" name="start_update" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">	
<input type="hidden" name="current_page" value="Advanced_FirmwareUpgrade_Content.asp">
<input type="hidden" name="next_page" value="Advanced_FirmwareUpgrade_Content.asp">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="">	
</form>
<form name="hint_form"></form>
</body>
</html>

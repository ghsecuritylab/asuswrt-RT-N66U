<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8">
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png"><title>ASUS Wireless Router <#Web_Title#> - <#menu2#></title>
<link rel="stylesheet" type="text/css" href="index_style.css">
<link rel="stylesheet" type="text/css" href="form_style.css">
<link rel="stylesheet" type="text/css" href="usp_style.css">
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/switcherplugin/jquery.iphone-switch.js"></script>
<script>
var $j = jQuery.noConflict();
</script>
<style type="text/css">
.upnp_table{
	width:750px;
	padding:5px; 
	padding-top:20px; 
	margin-top:-17px; 
	position:relative;
	background-color:#4d595d;
	align:left;
	-webkit-border-top-right-radius: 05px;
	-webkit-border-bottom-right-radius: 5px;
	-webkit-border-bottom-left-radius: 5px;
	-moz-border-radius-topright: 05px;
	-moz-border-radius-bottomright: 5px;
	-moz-border-radius-bottomleft: 5px;
	border-top-right-radius: 05px;
	border-bottom-right-radius: 5px;
	border-bottom-left-radius: 5px;
}
.line_export{
	height:20px;
	width:736px;
}
.upnp_button_table{
	width:730px;
	background-color:#15191b;
	margin-top:15px;
	margin-right:5px;
}
.upnp_button_table th{
	width:300px;
	height:40px;
	text-align:left;
	background-color:#1f2d35;
	font:Arial, Helvetica, sans-serif;
	font-size:12px;
	padding-left:10px;
	color:#FFFFFF;
	background: url(/images/general_th.gif) repeat;
}	
.upnp_button_table td{
	width:436px;
	height:40px;
	background-color:#475a5f;
	font:Arial, Helvetica, sans-serif;
	font-size:12px;
	padding-left:5px;
	color:#FFFFFF;
}	
.upnp_icon{
	background: url(/images/New_ui/media_sever.gif) no-repeat;
	width:736px;
	height:500px;
	margin-top:15px;
	margin-right:5px;
}
</style>
<script>
wan_route_x = '<% nvram_get("wan_route_x"); %>';
wan_nat_x = '<% nvram_get("wan_nat_x"); %>';
wan_proto = '<% nvram_get("wan_proto"); %>';

function initial(){
	show_menu();
	$("option5").innerHTML = '<img style="margin-left:10px;" border="0" width="50px" height="50px" src="images/New_ui/icon_index_5.png"><div style="margin-top:-30px; margin-left:65px"><#Menu_usb_application#></div>';
	$("option5").className = "m5_r";
}

function submit_mediaserver(server, x){
	var server_type = eval('document.mediaserverForm.'+server);

	showLoading();
	if(x == 1)
		server_type.value = 0;
	else
		server_type.value = 1;

	document.mediaserverForm.flag.value = "nodetect";	
	document.mediaserverForm.submit();
}
</script>
</head>

<body onload="initial();" onunload="unload_body();">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0" scrolling="no"></iframe>
<form method="post" name="mediaserverForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="restart_media">
<input type="hidden" name="action_wait" value="2">
<input type="hidden" name="current_page" value="/mediaserver.asp">
<input type="hidden" name="flag" value="">
<input type="hidden" name="dms_enable" value="<% nvram_get("dms_enable"); %>">
<input type="hidden" name="daapd_enable" value="<% nvram_get("daapd_enable"); %>">
</form>

<form method="post" name="form" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="">
</form>

<table class="content" align="center" cellspacing="0">
  <tr>
	<td width="17">&nbsp;</td>
	
	<!--=====Beginning of Main Menu=====-->
	<td valign="top" width="202">
	  <div id="mainMenu"></div>
	  <div id="subMenu"></div>
	</td>
	
  <td valign="top">
		<div id="tabMenu" class="submenuBlock"></div>
		<br>

<!--=====Beginning of Main Content=====-->
<div class="upnp_table">
<table>
  <tr>
  	<td>
				<div style="width:730px">
					<table width="730px">
						<tr>
							<td align="left">
								<span class="formfonttitle"><#UPnPMediaServer#></span>
							</td>
							<td align="right">
								<img onclick="go_setting('/APP_Installation.asp')" align="right" style="cursor:pointer;position:absolute;margin-left:-20px;margin-top:-30px;" title="Back to USB Extension" src="/images/backprev.png" onMouseOver="this.src='/images/backprevclick.png'" onMouseOut="this.src='/images/backprev.png'">
							</td>
						</tr>
					</table>
				</div>
				<div style="margin:5px;"><img src="/images/New_ui/export/line_export.png"></div>

			<div class="formfontdesc"><#upnp_Desc#></div>	
		</td>	<!--<span class="formfonttitle"></span> -->
  </tr>  
  <!--tr>
  	<td class="line_export"><img src="images/New_ui/export/line_export.png" /></td>
  </tr-->
   
  <tr>
   	<td>
   		<div class="upnp_button_table"> 
   		<table cellspacing="1">
   			<tr>
        	<th>Enabled DLNA Media Server:</th>
        	<td>
        			<div class="left" style="width:94px; position:relative; left:3%;" id="radio_dms_enable"></div>
							<div class="clear"></div>
							<script type="text/javascript">
									$j('#radio_dms_enable').iphoneSwitch('<% nvram_get("dms_enable"); %>', 
										 function() {
											submit_mediaserver("dms_enable", 0);
										 },
										 function() {
											submit_mediaserver("dms_enable", 1);
										 },
										 {
											switch_on_container_path: '/switcherplugin/iphone_switch_container_off.png'
										 }
									);
							</script>
        		</td>
       	</tr>

   			<tr>
        	<th>Enabled iTunes Server:</th>
        	<td>
        			<div class="left" style="width:94px; position:relative; left:3%;" id="radio_daapd_enable"></div>
							<div class="clear"></div>
							<script type="text/javascript">
									$j('#radio_daapd_enable').iphoneSwitch('<% nvram_get("daapd_enable"); %>', 
										 function() {
											submit_mediaserver("daapd_enable", 0);
										 },
										 function() {
											submit_mediaserver("daapd_enable", 1);
										 },
										 {
											switch_on_container_path: '/switcherplugin/iphone_switch_container_off.png'
										 }
									);
							</script>
        		</td>
       	</tr>

      	</table> 
      	</div>
    	</td> 
  </tr>  
  
  <tr>
  	<td>
  	<div class="upnp_icon"></div>
  	</td>
  </tr>
</table>

<!--=====End of Main Content=====-->
		</td>

		<td width="20" align="center" valign="top"></td>
	</tr>
</table>
</div>

<div id="footer"></div>
</body>
</html>


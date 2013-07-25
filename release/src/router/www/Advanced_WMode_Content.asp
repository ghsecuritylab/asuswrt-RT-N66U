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
<title><#Web_Title#> - <#menu5_1_3#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<style>
#pull_arrow{
 	float:center;
 	cursor:pointer;
 	border:2px outset #EFEFEF;
 	background-color:#CCC;
 	padding:3px 2px 4px 0px;
}
#WDSAPList{
	border:1px outset #999;
	background-color:#576D73;
	position:absolute;
	margin-top:24px;
	margin-left:220px;
	*margin-left:-150px;
	width:255px;
	*width:259px;
	text-align:left;
	height:auto;
	overflow-y:auto;
	z-index:200;
	padding: 1px;
	display:none;
}
#WDSAPList div{
	background-color:#576D73;
	height:20px;
	line-height:20px;
	text-decoration:none;
	padding-left:2px;
}
#WDSAPList a{
	background-color:#EFEFEF;
	color:#FFF;
	font-size:12px;
	font-family:Arial, Helvetica, sans-serif;
	text-decoration:none;	
}
#WDSAPList div:hover, #ClientList_Block a:hover{
	background-color:#3366FF;
	color:#FFFFFF;
	cursor:default;
}
</style>
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script type="text/javascript" language="JavaScript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/detect.js"></script>
<script language="JavaScript" type="text/JavaScript" src="/jquery.js"></script>
<script>
wan_route_x = '<% nvram_get("wan_route_x"); %>';
wan_nat_x = '<% nvram_get("wan_nat_x"); %>';
wan_proto = '<% nvram_get("wan_proto"); %>';

<% login_state_hook(); %>
<% wl_get_parameter(); %>

var wireless = [<% wl_auth_list(); %>];	// [[MAC, associated, authorized], ...]
var wl_wdslist_array = '<% nvram_get("wl_wdslist"); %>';
var wds_aplist = "";
var $j = jQuery.noConflict();

function initial(){
	show_menu();
	load_body();	
	insertExtChannelOption();
	document.form.wl_channel.value = document.form.wl_channel_orig.value;	

	if(sw_mode == 2 && '<% nvram_get("wl_unit"); %>' == '<% nvram_get("wlc_band"); %>'){
		for(var i=5; i>=3; i--)
			$("MainTable1").deleteRow(i);
		for(var i=2; i>=0; i--)
			$("MainTable2").deleteRow(i);
		$("repeaterModeHint").style.display = "";
		$("wl_wdslist_Block").style.display = "none";
		$("submitBtn").style.display = "none";
	}
	else
		show_wl_wdslist();

	if(band5g_support == -1){
		$("wl_unit_field").style.display = "none";
		$("mac_5g").style.display = "none";
	}	

	setTimeout("wds_scan();", 500);
}

function show_wl_wdslist(){
	var wl_wdslist_row = wl_wdslist_array.split('&#60');
	var code = "";
	code +='<table width="100%" border="1" cellspacing="0" cellpadding="4" align="center" class="list_table" id="wl_wdslist_table">'; 
	if(wl_wdslist_row.length == 1){
		code +='<tr><td style="color:#FFCC00;"><#IPConnection_VSList_Norule#></td>';
	}		
	else{
		for(var i = 1; i < wl_wdslist_row.length; i++){
			code +='<tr id="row'+i+'">';
			code +='<td width="80%">'+ wl_wdslist_row[i] +'</td>';	
			code +='<td width="20%"><input type="button" class=\"remove_btn\" onclick=\"deleteRow(this);\" value=\"\"/></td></tr>';
		}
	}
  code +='</tr></table>';	
	$("wl_wdslist_Block").innerHTML = code;
}

function deleteRow(r){
  var i=r.parentNode.parentNode.rowIndex;
  $('wl_wdslist_table').deleteRow(i);
  
  var wl_wdslist_value = "";
	for(i=0; i<$('wl_wdslist_table').rows.length; i++){
		wl_wdslist_value += "&#60";
		wl_wdslist_value += $('wl_wdslist_table').rows[i].cells[0].innerHTML;
	}
	
	wl_wdslist_array = wl_wdslist_value;
	if(wl_wdslist_array == "")
		show_wl_wdslist();
}

function addRow(obj, upper){
	var rule_num = $('wl_wdslist_table').rows.length;
	var item_num = $('wl_wdslist_table').rows[0].cells.length;
	
	if(rule_num >= upper){
		alert("<#JS_itemlimit1#> " + upper + " <#JS_itemlimit2#>");
		return false;	
	}		
		
	if(obj.value==""){
		alert("<#JS_fieldblank#>");
		obj.focus();
		obj.select();
		return false;
	}else if (!check_macaddr(obj, check_hwaddr_flag(obj))){
		obj.focus();
		obj.select();		
		return false;
	}
		
		//Viz check same rule
		for(i=0; i<rule_num; i++){
			for(j=0; j<item_num-1; j++){	
				if(obj.value.toLowerCase() == $('wl_wdslist_table').rows[i].cells[j].innerHTML.toLowerCase()){
					alert("<#JS_duplicate#>");
					return false;
				}	
			}		
		}
		
		wl_wdslist_array += "&#60";
		wl_wdslist_array += obj.value;
		obj.value = "";
		show_wl_wdslist();
	
}

function applyRule(){
	var rule_num = $('wl_wdslist_table').rows.length;
	var item_num = $('wl_wdslist_table').rows[0].cells.length;
	var tmp_value = "";

	for(i=0; i<rule_num; i++){
		tmp_value += "<"		
		for(j=0; j<item_num-1; j++){	
			tmp_value += $('wl_wdslist_table').rows[i].cells[j].innerHTML;
			if(j != item_num-2)	
				tmp_value += ">";
		}
	}
	if(tmp_value == "<"+"<#IPConnection_VSList_Norule#>" || tmp_value == "<")
		tmp_value = "";	
	
	document.form.wl_wdslist.value = tmp_value;

	if(document.form.wl_mode_x.value == "0"){
		inputRCtrl1(document.form.wl_wdsapply_x, 1);
		inputRCtrl2(document.form.wl_wdsapply_x, 1);
	}
	
	if(document.form.wl_mode_x.value == "1")
		inputRCtrl1(document.form.wl_wdsapply_x, 1);

	showLoading();	
	document.form.submit();
}

function done_validating(action){
	refreshpage();
}

/*------------ Site Survey Start -----------------*/
function wds_scan(){
	var ajaxURL = '/wds_aplist_2g.asp';
	if('<% nvram_get("wl_unit"); %>' == '1')
		var ajaxURL = '/wds_aplist_5g.asp';

	$j.ajax({
		url: ajaxURL,
		dataType: 'script',
		
		error: function(xhr){
			setTimeout("wds_scan();", 1000);
		},
		success: function(response){
			showLANIPList();
		}
	});
}

function setClientIP(num){
	document.form.wl_wdslist_0.value = wds_aplist[num][1];
	hideClients_Block();
	over_var = 0;
}

function rescan(){
	wds_aplist = "";
	showLANIPList()
	wds_scan();
}

function showLANIPList(){
	var code = "";
	var show_name = "";
	if(wds_aplist != ""){
		for(var i = 0; i < wds_aplist.length ; i++){
			wds_aplist[i][0] = decodeURIComponent(wds_aplist[i][0]);
			if(wds_aplist[i][0] && wds_aplist[i][0].length > 12)
				show_name = wds_aplist[i][0].substring(0, 10) + "..";
			else
				show_name = wds_aplist[i][0];
			
			if(wds_aplist[i][1]){
				code += '<a><div onmouseover="over_var=1;" onmouseout="over_var=0;" onclick="setClientIP('+i+');"><strong>'+show_name+'</strong>';
				if(show_name && show_name.length > 0)
					code += '( '+wds_aplist[i][1]+')';
				else
					code += wds_aplist[i][1];
				code += ' </div></a>';
			}
		}
		code += '<div style="text-decoration:underline;font-weight:bolder;" onclick="rescan();"><#AP_survey#></div>';
	}
	else{
		code += '<div style="width:98px"><img height="15px" style="margin-left:5px;margin-top:2px;" src="/images/InternetScan.gif"></div>';
	}

	code +='<!--[if lte IE 6.5]><iframe class="hackiframe2"></iframe><![endif]-->';	
	document.getElementById("WDSAPList").innerHTML = code;
}

function pullLANIPList(obj){	
	if(isMenuopen == 0){		
		obj.src = "/images/arrow-top.gif"
		document.getElementById("WDSAPList").style.display = 'block';		
		document.form.wl_wdslist_0.focus();		
		isMenuopen = 1;
	}
	else
		hideClients_Block();
}
var over_var = 0;
var isMenuopen = 0;

function hideClients_Block(){
	document.getElementById("pull_arrow").src = "/images/arrow-down.gif";
	document.getElementById('WDSAPList').style.display='none';
	isMenuopen = 0;
}

function check_macaddr(obj,flag){ //control hint of input mac address
	if(flag == 1){
		var childsel=document.createElement("div");
		childsel.setAttribute("id","check_mac");
		childsel.style.color="#FFCC00";
		obj.parentNode.appendChild(childsel);
		$("check_mac").innerHTML="<br><br><#LANHostConfig_ManualDHCPMacaddr_itemdesc#>";
		$("check_mac").style.display = "";
		return false;	
	}else if(flag == 2){
		var childsel=document.createElement("div");
		childsel.setAttribute("id","check_mac");
		childsel.style.color="#FFCC00";
		obj.parentNode.appendChild(childsel);
		$("check_mac").innerHTML="<br><br><#IPConnection_x_illegal_mac#>";
		$("check_mac").style.display = "";
		return false;			
	}else{
		$("check_mac") ? $("check_mac").style.display="none" : true;
		return true;
	} 	
}
/*---------- Site Survey End -----------------*/
</script>
</head>

<body onload="initial();" onunLoad="return unload_body();">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" value="<% nvram_get("productid"); %>" name="productid" >
<input type="hidden" value="<% nvram_get("wan_route_x"); %>" name="wan_route_x" >
<input type="hidden" value="<% nvram_get("wan_nat_x"); %>" name="wan_nat_x" >

<input type="hidden" name="current_page" value="Advanced_WMode_Content.asp">
<input type="hidden" name="next_page" value="Advanced_WMode_Content.asp">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="group_id" value="wl_wdslist">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_wait" value="3">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="action_script" value="restart_wireless">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">

<input type="hidden" maxlength="15" size="15" name="x_RegulatoryDomain" value="<% nvram_get("x_RegulatoryDomain"); %>" readonly="1">
<input type="hidden" name="wl_wdsnum_x_0" value="<% nvram_get("wl_wdsnum_x"); %>" readonly="1">  
<input type="hidden" name="wl_channel_orig" value='<% nvram_get("wl_channel"); %>'>
<input type="hidden" name="wl_nmode_x" value='<% nvram_get("wl_nmode_x"); %>'>
<input type="hidden" name="wl_bw" value='<% nvram_get("wl_bw"); %>'>
<input type="hidden" name="wl_nctrlsb_old" value='<% nvram_get("wl_nctrlsb"); %>'>
<input type="hidden" name="wl_wdslist" value=''>
<input type="hidden" name="wl_subunit" value="-1">

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
		
<table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">
<tbody>
		<tr>
		  	<td bgcolor="#4D595D" valign="top">
		  	<div>&nbsp;</div>
		  	<div class="formfonttitle"><#menu5_1#> - <#menu5_1_3#></div>
		  	<div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
		  	<div class="formfontdesc"><#WLANConfig11b_display3_sectiondesc#></div>
		  	<div style="margin-left:10px;">(2.4GHz MAC) <% nvram_get("wl0_hwaddr"); %></div>
		  	<div id="mac_5g" style="margin-left:10px;">(5GHz MAC) <% nvram_get("wl1_hwaddr"); %></div>
			
			<table id="MainTable1" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
			  <thead>
			  <tr>
				<td colspan="2"><#t2BC#></td>
			  </tr>
			  </thead>		

				<tr id="wl_unit_field">
					<th><#Interface#></th>
					<td>
						<select name="wl_unit" class="input_option" onChange="change_wl_unit();">
							<option class="content_input_fd" value="0" <% nvram_match("wl_unit", "0","selected"); %>>2.4GHz</option>
							<option class="content_input_fd" value="1"<% nvram_match("wl_unit", "1","selected"); %>>5GHz</option>
						</select>			
					</td>
			  </tr>

				<tr id="repeaterModeHint" style="display:none;">
					<td colspan="2" style="color:#FFCC00;height:30px;" align="center"><#page_not_support_mode_hint#></td>
			  </tr>
			
				<tr>
					<th align="right" >
						<a class="hintstyle" href="javascript:void(0);"  onClick="openHint(1,1);">
						<#WLANConfig11b_x_APMode_itemname#></a>
					</th>
					<td>
						<select name="wl_mode_x" class="input_option" onChange="return change_common(this, 'WLANConfig11b', 'wl_mode_x');">
							<option value="0" <% nvram_match("wl_mode_x", "0","selected"); %>>AP Only</option>
							<option value="1" <% nvram_match("wl_mode_x", "1","selected"); %>>WDS Only</option>
							<option value="2" <% nvram_match("wl_mode_x", "2","selected"); %>>Hybrid</option>
					  	</select>
					</td>
				</tr>

				<tr>
					<th align="right">
						<a class="hintstyle" href="javascript:void(0);"  onClick="openHint(1,3);">
						<#WLANConfig11b_x_BRApply_itemname#>
						</a>
					</th>
					<td>
						<input type="radio" value="1" name="wl_wdsapply_x" class="input" onClick="return change_common_radio(this, 'WLANConfig11b', 'wl_wdsapply_x', '1')" <% nvram_match("wl_wdsapply_x", "1", "checked"); %>><#checkbox_Yes#>
						<input type="radio" value="0" name="wl_wdsapply_x" class="input" onClick="return change_common_radio(this, 'WLANConfig11b', 'wl_wdsapply_x', '0')" <% nvram_match("wl_wdsapply_x", "0", "checked"); %>><#checkbox_No#>
					</td>
				</tr>			

				<tr>
					<th align="right">
						<a class="hintstyle"href="javascript:void(0);"  onClick="openHint(1,2);">
						<#WLANConfig11b_Channel_itemname#>
						</a>
					</th>
					<td>
						<select name="wl_channel" class="input_option" onChange="return change_common(this, 'WLANConfig11b', 'wl_channel')">
						<% select_channel("WLANConfig11b"); %>
						</select>
					</td>
				</tr>
	
			  <tr style="display:none;">
				 	<th><#WLANConfig11b_EChannel_itemname#></th>
			  	<td>
					<select name="wl_nctrlsb">
						<option value="lower" <% nvram_match("wl_nctrlsb", "lower", "selected"); %>>lower</option>
						<option value="upper"<% nvram_match("wl_nctrlsb", "upper", "selected"); %>>upper</option>
					</select>
					</td>
			  </tr>
			</table>
			
			<table id="MainTable2" width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable_table">
			  <thead>
			  <tr>
				<td colspan="4"><#WLANConfig11b_RBRList_groupitemdesc#></td>
			  </tr>
			  </thead>		

          		<tr>
            		<th width="80%"><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,10);">
								 <#WLANConfig11b_RBRList_groupitemdesc#>
								</th>
								<th class="edit_table" width="20%">Add / Delete</th>
          		</tr>
          		<tr>
            		<td width="80%">
              		<input type="text" style="margin-left:220px;float:left;" maxlength="17" class="input_macaddr_table" name="wl_wdslist_0" onKeyPress="return is_hwaddr(this,event)">
									<img style="float:left;" id="pull_arrow" height="14px;" src="/images/arrow-down.gif" onclick="pullLANIPList(this);" title="Select the Access Point" onmouseover="over_var=1;" onmouseout="over_var=0;">
									<div id="WDSAPList" class="WDSAPList">
										<div style="width:98px">
											<img height="15px" style="margin-left:5px;margin-top:2px;" src="/images/InternetScan.gif">
										</div>
									</div>
              	</td>
              	<td width="20%">	
              		<input type="button" class="add_btn" onClick="addRow(document.form.wl_wdslist_0, 4);" value="">
              	</td>
          		</tr>
        		</table>
        		
          			<div id="wl_wdslist_Block"></div>
          		
				<div class="apply_gen">
					<input class="button_gen" id="submitBtn" onclick="applyRule()" type="button" value="<#CTL_apply#>" />
				</div>        		

        	</td>
	</tr>
	</tbody>	
</table>


</td>
</form>
		
  </tr>
</table>				
<!--===================================Ending of Main Content===========================================-->		
	
	</td>
		
    <td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>

<div id="footer"></div>
</body>
</html>

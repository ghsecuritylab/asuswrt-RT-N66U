﻿<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<head>
<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7">
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>Network Tools - Wake On LAN</title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<style>
#ClientList_Block_PC{
	border:1px outset #999;
	background-color:#576D73;
	position:absolute;
	margin-top:103px;
	*margin-top:96px;	
	margin-left:59px;
	width:255px;
	text-align:left;	
	height:auto;
	overflow-y:auto;
	z-index:200;
	padding: 1px;
	display:none;
}
#ClientList_Block_PC div{
	background-color:#576D73;
	height:auto;
	*height:20px;
	line-height:20px;
	text-decoration:none;
	font-family: Lucida Console;
	padding-left:2px;
}

#ClientList_Block_PC a{
	background-color:#EFEFEF;
	color:#FFF;
	font-size:12px;
	font-family:Arial, Helvetica, sans-serif;
	text-decoration:none;	
}
#ClientList_Block_PC div:hover, #ClientList_Block a:hover{
	background-color:#3366FF;
	color:#FFFFFF;
	cursor:default;
}	
</style>	
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script type="text/javascript" language="JavaScript" src="/detect.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/jquery.js"></script>
<script>
function onSubmitCtrl(o, s) {
	if(document.form.destIP.value == ""){
		alert("<#JS_fieldblank#>");
		document.form.destIP.focus();
		return false;
	}

	if(check_hwaddr_flag(document.form.destIP) != 0){
		alert("<#IPConnection_x_illegal_mac#>");
		document.form.destIP.focus();
		return false;
	}

	document.form.action_mode.value = s;
	updateOptions();
}

function updateOptions(){
	document.form.SystemCmd.value = "ether-wake -i br0 " + document.form.destIP.value;
	document.form.submit();
	document.getElementById("cmdBtn").disabled = true;
	document.getElementById("cmdBtn").style.color = "#666";
	document.getElementById("loadingIcon").style.display = "";
	setTimeout("checkCmdRet();", 500);
}

var $j = jQuery.noConflict();
var _responseLen;
var noChange = 0;
function checkCmdRet(){
	$j.ajax({
		url: '/cmdRet_check.htm',
		dataType: 'html',
		
		error: function(xhr){
			setTimeout("checkCmdRet();", 1000);
		},
		success: function(response){
			if(response.search("XU6J03M6") != -1){
				document.getElementById("loadingIcon").style.display = "none";
				document.getElementById("cmdBtn").disabled = false;
				document.getElementById("cmdBtn").style.color = "#FFF";
				return false;
			}

			if(_responseLen == response.length)
				noChange++;
			else
				noChange = 0;

			if(noChange > 5){
				document.getElementById("loadingIcon").style.display = "none";
				document.getElementById("cmdBtn").disabled = false;
				document.getElementById("cmdBtn").style.color = "#FFF";
				setTimeout("checkCmdRet();", 1000);
			}
			else{
				document.getElementById("cmdBtn").disabled = true;
				document.getElementById("cmdBtn").style.color = "#666";
				document.getElementById("loadingIcon").style.display = "";
				setTimeout("checkCmdRet();", 1000);
			}

			_responseLen = response.length;
		}
	});
}

var wollist_array = '<% nvram_get("wollist"); %>';
function showwollist(){
	var wollist_row = wollist_array.split('&#60');
	var code = "";

	code +='<table width="100%" cellspacing="0" cellpadding="4" align="center" class="list_table" id="wollist_table">';
	if(wollist_row.length == 1)
		code +='<tr><td style="color:#FFCC00;" colspan="6"><#IPConnection_VSList_Norule#></td></tr>';
	else{
		for(var i = 1; i < wollist_row.length; i++){
			code +='<tr id="row'+i+'">';
			var wollist_col = wollist_row[i].split('&#62');
				for(var j = 0; j < wollist_col.length; j++){
					if(j == 0)
						code +='<td width="40%">'+ wollist_col[j] +'</td>';
					else
						code +='<td width="40%" style="font-weight:bold;cursor:pointer;text-decoration:underline;font-family:Lucida Console;" onclick="document.form.destIP.value=this.innerHTML;">'+ wollist_col[j] +'</td>';
				}
				code +='<td width="20%">';
				code +='<input class="remove_btn" onclick="del_Row(this);" value=""/></td></tr>';
		}
	}

  code +='</table>';
	$("wollist_Block").innerHTML = code;
}

function addRow(obj, head){

	if(head == 1)
		wollist_array += "&#60" /*&#60*/
	else
		wollist_array += "&#62" /*&#62*/
			
	wollist_array += obj.value;

	obj.value = "";
}

function addRow_Group(upper){
	var rule_num = $('wollist_table').rows.length;
	var item_num = $('wollist_table').rows[0].cells.length;		
	if(rule_num >= upper){
		alert("<#JS_itemlimit1#> " + upper + " <#JS_itemlimit2#>");
		return false;	
	}			

	if(item_num >=2){
		for(i=0; i<rule_num; i++){	
				if(document.form.wollist_deviceName.value.toLowerCase() == $('wollist_table').rows[i].cells[0].innerHTML.toLowerCase()){
					alert("<#JS_duplicate#>");
					document.form.wollist_deviceName.focus();
					document.form.wollist_deviceName.select();
					return false;
				}	
		}
	}		
	
	addRow(document.form.wollist_deviceName ,1);
	addRow(document.form.wollist_macAddr, 0);
	showwollist();				
}

function del_Row(r){
  var i=r.parentNode.parentNode.rowIndex;
  $('wollist_table').deleteRow(i);
  
  var wollist_value = "";
	for(k=0; k<$('wollist_table').rows.length; k++){
		for(j=0; j<$('wollist_table').rows[k].cells.length-1; j++){
			if(j == 0)	
				wollist_value += "<";
			else{
			wollist_value += $('wollist_table').rows[k].cells[0].firstChild.innerHTML;
			wollist_value += ">";
			wollist_value += $('wollist_table').rows[k].cells[1].firstChild.innerHTML;
			}
		}
	}

	wollist_array = wollist_value;
	if(wollist_array == "")
		showwollist();
}

function showLANIPList(){
	var code = "";
	var show_name = "";
	var client_list_array = '<% get_client_detail_info(); %>';	
	var client_list_row = client_list_array.split('<');	

	for(var i = 1; i < client_list_row.length; i++){
		var client_list_col = client_list_row[i].split('>');
		if(client_list_col[1] && client_list_col[1].length > 20)
			show_name = client_list_col[1].substring(0, 16) + "..";
		else
			show_name = client_list_col[1];	

		code += '<a><div onmouseover="over_var=1;" onmouseout="over_var=0;" onclick="setClientIP(\''+client_list_col[1]+'\', \''+client_list_col[3]+'\');"><strong>'+client_list_col[3]+'</strong> ';
		
		if(show_name && show_name.length > 0)
				code += '( '+show_name+')';
		code += ' </div></a>';
	}

	code +='<!--[if lte IE 6.5]><iframe class="hackiframe2"></iframe><![endif]-->';	
	$("ClientList_Block_PC").innerHTML = code;
}

function setClientIP(_name, _macaddr){
	document.form.wollist_deviceName.value = _name;
	document.form.wollist_macAddr.value = _macaddr;
	hideClients_Block();
	over_var = 0;
}

var over_var = 0;
var isMenuopen = 0;
function hideClients_Block(){
	$("pull_arrow").src = "/images/arrow-down.gif";
	$('ClientList_Block_PC').style.display='none';
	isMenuopen = 0;
}

function pullLANIPList(obj){
	if(isMenuopen == 0){		
		obj.src = "/images/arrow-top.gif"
		$("ClientList_Block_PC").style.display = 'block';		
		document.form.wollist_deviceName.focus();		
		isMenuopen = 1;
	}
	else
		hideClients_Block();
}

function applyRule(){
	var rule_num = $('wollist_table').rows.length;
	var item_num = $('wollist_table').rows[0].cells.length;
	var tmp_value = "";

	for(i=0; i<rule_num; i++){
		tmp_value += "<"		
		for(j=0; j<item_num-1; j++){	
			tmp_value += $('wollist_table').rows[i].cells[j].innerHTML;
			if(j != item_num-2)	
				tmp_value += ">";
		}
	}
	if(tmp_value == "<"+"<#IPConnection_VSList_Norule#>" || tmp_value == "<")
		tmp_value = "";	

	document.wolform.wollist.value = tmp_value;
	showLoading();
	document.wolform.submit();
}
</script>
</head>
<body onload="show_menu();load_body();showwollist();showLANIPList();">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="GET" name="form" action="/apply.cgi" target="hidden_frame"> 
<input type="hidden" name="current_page" value="Main_Netstat_Content.asp">
<input type="hidden" name="next_page" value="Main_Netstat_Content.asp">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="group_id" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="">
<input type="hidden" name="action_script" value="">
<input type="hidden" name="action_wait" value="">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="SystemCmd" onkeydown="onSubmitCtrl(this, ' Refresh ')" value="">
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
			<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
				<tr>
					<td align="left" valign="top">				
						<table width="760px" border="0" cellpadding="5" cellspacing="0" bordercolor="#6b8fa3"  class="FormTitle" id="FormTitle">		
							<tr>
								<td bgcolor="#4D595D" colspan="3" valign="top">
									<div>&nbsp;</div>
									<div class="formfonttitle">Network Tools - Wake On LAN</div>
									<div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
									<div class="formfontdesc">
										Send a magic packet to wake up sleeping machines.
										MAC must be a station address (00:11:22:33:44:55) or
										a hostname with a known 'ethers' entry.
									</div>
									<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
										<tr>
											<th width="20%">Target</th>
											<td>
												<input type="text" class="input_20_table" maxlength="17" name="destIP" value="" placeholder="ex: <% nvram_get("et0macaddr"); %>" onKeyPress="return is_hwaddr(this,event);">
												<input class="button_gen" id="cmdBtn" onClick="onSubmitCtrl(this, ' Refresh ')" type="button" value="Wake" name="action">
												<img id="loadingIcon" style="display:none;" src="/images/InternetScan.gif"></span>
											</td>										
										</tr>
									</table>

									<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table" style="margin-top:8px;">
									  	<thead>
									  		<tr>
												<td colspan="3" id="GWStatic">Offline list</td><!--Viz hold on this : &nbsp;&nbsp;(<#List_limit#> 32)-->
									  		</tr>
									  	</thead>
						
									  	<tr>
						        		<th><#ShareNode_DeviceName_itemname#></th>
								  			<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(5,10);"><#MAC_Address#></a></th>
						        		<th>Add / Delete</th>
									  	</tr>			  
									  	<tr>
									  			<!-- client info -->
													<div id="ClientList_Block_PC" class="ClientList_Block_PC"></div>									  		
				            			<td width="40%">
				            				<input type="text" class="input_20_table" maxlength="15" name="wollist_deviceName" onClick="hideClients_Block();">
				                		<img id="pull_arrow" height="14px;" src="/images/arrow-down.gif" style="position:absolute;" onclick="pullLANIPList(this);" title="Select the device name of DHCP clients." onmouseover="over_var=1;" onmouseout="over_var=0;">
				            			</td>
				            			<td width="40%">
				                		<input type="text" class="input_20_table" maxlength="17" name="wollist_macAddr" style="" onKeyPress="return is_hwaddr(this,event)">
		                			</td>
				            			<td width="20%">
														<div> 
															<input type="button" class="add_btn" onClick="addRow_Group(32);" value="">
														</div>
				            			</td>
									  	</tr>	 			  
									  </table>        			

								  <div id="wollist_Block"></div>

			           	<div class="apply_gen">
			           		<input type="button" name="button" class="button_gen" onclick="applyRule();" value="<#CTL_apply#>"/>
		            	</div>
								</td>
							</tr>
						</table>
					</td>
				</tr>
			</table>
			<!--===================================Ending of Main Content===========================================-->
		</td>
		<td width="10" align="center" valign="top"></td>
	</tr>
</table>
</form>

<form method="post" name="wolform" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="productid" value="<% nvram_get("productid"); %>">
<input type="hidden" name="current_page" value="Main_WOL_Content.asp">
<input type="hidden" name="next_page" value="Main_WOL_Content.asp">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_wait" value="3">
<input type="hidden" name="action_script" value="restart_time">
<input type="hidden" name="wollist" value="<% nvram_get("wollist"); %>">
</form>

<div id="footer"></div>
</body>
</html>

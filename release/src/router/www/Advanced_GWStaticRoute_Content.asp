<!DOCTYPE html PUBLIC "-//W3C//DTD XHTML 1.0 Transitional//EN" "http://www.w3.org/TR/xhtml1/DTD/xhtml1-transitional.dtd">
<html xmlns="http://www.w3.org/1999/xhtml">
<html xmlns:v>
<head>
<meta http-equiv="X-UA-Compatible" content="IE=EmulateIE7"/>
<meta http-equiv="Content-Type" content="text/html; charset=utf-8" />
<meta HTTP-EQUIV="Pragma" CONTENT="no-cache">
<meta HTTP-EQUIV="Expires" CONTENT="-1">
<link rel="shortcut icon" href="images/favicon.png">
<link rel="icon" href="images/favicon.png">
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_2_3#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" language="JavaScript" src="/help.js"></script>
<script type="text/javascript" language="JavaScript" src="/detect.js"></script>

<script>
wan_route_x = '<% nvram_get("wan_route_x"); %>';
wan_nat_x = '<% nvram_get("wan_nat_x"); %>';
wan_proto = '<% nvram_get("wan_proto"); %>';

<% login_state_hook(); %>
var wireless = [<% wl_auth_list(); %>];	// [[MAC, associated, authorized], ...]
var sr_rulelist_array = '<% nvram_get("sr_rulelist"); %>';

function initial(){
	show_menu();
	showsr_rulelist();
}

function applyRule(){
	var rule_num = $('sr_rulelist_table').rows.length;
	var item_num = $('sr_rulelist_table').rows[0].cells.length;
	var tmp_value = "";

	for(i=0; i<rule_num; i++){
		tmp_value += "<"		
		for(j=0; j<item_num-1; j++){	
			tmp_value += $('sr_rulelist_table').rows[i].cells[j].innerHTML;
			if(j != item_num-2)	
				tmp_value += ">";
		}
	}
	if(tmp_value == "<"+"<#IPConnection_VSList_Norule#>" || tmp_value == "<")
		tmp_value = "";	

	document.form.sr_rulelist.value = tmp_value;

	showLoading();
	document.form.submit();
}

function done_validating(action){
	refreshpage();
}

function GWStatic_validate_duplicate_noalert(o, v, l, off){	
	for (var i=0; i<o.length; i++)
	{
		if (entry_cmp(o[i][off], v, l)==0){ 
			return true;
		}
	}
	return false;
}

function GWStatic_validate_duplicate(o, v, l, off){
	for(var i = 0; i < o.length; i++){
		if(entry_cmp(o[i][off].toLowerCase(), v.toLowerCase(), l) == 0){
			alert('<#JS_duplicate#>');
			return true;
		}
	}
	return false;
}

// start new table func. // jerry5 added.
function addRow(obj, head){
	if(head == 1)
		sr_rulelist_array += "&#60"
	else
		sr_rulelist_array += "&#62"
			
	sr_rulelist_array += obj.value;
	obj.value = "";
}

function addRow_Group(upper){
	var rule_num = $('sr_rulelist_table').rows.length;
	var item_num = $('sr_rulelist_table').rows[0].cells.length;	
	
	if(rule_num >= upper){
		alert("<#JS_itemlimit1#> " + upper + " <#JS_itemlimit2#>");
		return false;	
	}	
		
	if(document.form.sr_ipaddr_x_0.value==""){
		alert("<#JS_fieldblank#>");
		document.form.sr_ipaddr_x_0.focus();
		document.form.sr_ipaddr_x_0.select();		
		return false;
	}else	if(document.form.sr_netmask_x_0.value==""){
		alert("<#JS_fieldblank#>");
		document.form.sr_netmask_x_0.focus();
		document.form.sr_netmask_x_0.select();		
		return false;
	}else	if(document.form.sr_gateway_x_0.value==""){
		alert("<#JS_fieldblank#>");
		document.form.sr_gateway_x_0.focus();
		document.form.sr_gateway_x_0.select();		
		return false;
	}else if(valid_IP_form(document.form.sr_ipaddr_x_0, 0) && valid_IP_form(document.form.sr_gateway_x_0, 0)){	
		
		//2011.11 Viz add to valid netmask { start	// test if netmask is valid.
		var default_netmask = "";
		var wrong_netmask = 0;
		var netmask_obj = document.form.sr_netmask_x_0;
		var netmask_num = inet_network(netmask_obj.value);
		
		if(netmask_num==0){
			var netmask_reverse_num = 0;		//Viz 2011.07 : Let netmask 0.0.0.0 pass
		}else{
		var netmask_reverse_num = ~netmask_num;
		}
		
		if(netmask_num < 0) wrong_netmask = 1;

		var test_num = netmask_reverse_num;
		while(test_num != 0){
			if((test_num+1)%2 == 0)
				test_num = (test_num+1)/2-1;
			else{
				wrong_netmask = 1;
				break;
			}
		}
		if(wrong_netmask == 1){
			alert(netmask_obj.value+" <#JS_validip#>");
			netmask_obj.value = default_netmask;
			netmask_obj.focus();
			netmask_obj.select();
			return false;
		}
		//2011.11 Viz add to valid netmask } end		
		

//Viz check same rule  //match(ip+netmask) is not accepted 
		if(item_num >=2){
			for(i=0; i<rule_num; i++){
					if(document.form.sr_ipaddr_x_0.value == $('sr_rulelist_table').rows[i].cells[0].innerHTML 
						&& document.form.sr_gateway_x_0.value == $('sr_rulelist_table').rows[i].cells[2].innerHTML){
						alert("<#JS_duplicate#>");						
						document.form.sr_gateway_x_0.value="";
						document.form.sr_ipaddr_x_0.focus();
						document.form.sr_ipaddr_x_0.select();
						return false;
					}				
			}
		}		
		
		
		addRow(document.form.sr_ipaddr_x_0 ,1);
		addRow(document.form.sr_netmask_x_0, 0);
		addRow(document.form.sr_gateway_x_0, 0);
		addRow(document.form.sr_matric_x_0, 0);
		addRow(document.form.sr_if_x_o, 0);
		document.form.sr_if_x_o.value="LAN";
		showsr_rulelist();
	}else{
		return false;
	}	
}	

function edit_Row(r){ 	
	var i=r.parentNode.parentNode.rowIndex;
  	
	document.form.sr_ipaddr_x_0.value = $('sr_rulelist_table').rows[i].cells[0].innerHTML;
	document.form.sr_netmask_x_0.value = $('sr_rulelist_table').rows[i].cells[1].innerHTML; 
	document.form.sr_gateway_x_0.value = $('sr_rulelist_table').rows[i].cells[2].innerHTML; 
	document.form.sr_matric_x_0.value = $('sr_rulelist_table').rows[i].cells[3].innerHTML;
	document.form.sr_if_x_o.value = $('sr_rulelist_table').rows[i].cells[3].innerHTML;
	
  del_Row(r);	
}

function del_Row(r){
  var i=r.parentNode.parentNode.rowIndex;
  $('sr_rulelist_table').deleteRow(i);
  
  var sr_rulelist_value = "";
	for(k=0; k<$('sr_rulelist_table').rows.length; k++){
		for(j=0; j<$('sr_rulelist_table').rows[k].cells.length-1; j++){
			if(j == 0)	
				sr_rulelist_value += "&#60";
			else
				sr_rulelist_value += "&#62";
			sr_rulelist_value += $('sr_rulelist_table').rows[k].cells[j].innerHTML;		
		}
	}
	
	sr_rulelist_array = sr_rulelist_value;
	if(sr_rulelist_array == "")
		showsr_rulelist();
}

function showsr_rulelist(){
	var sr_rulelist_row = sr_rulelist_array.split('&#60');
	var code = "";

	code +='<table width="100%" border="1" cellspacing="0" cellpadding="4" align="center" class="list_table" id="sr_rulelist_table">';
	if(sr_rulelist_row.length == 1)
		code +='<tr><td style="color:#FFCC00;" colspan="6"><#IPConnection_VSList_Norule#></td></tr>';
	else{
		for(var i = 1; i < sr_rulelist_row.length; i++){
			code +='<tr id="row'+i+'">';
			var sr_rulelist_col = sr_rulelist_row[i].split('&#62');
			var wid=[20, 20, 20, 8, 10];
				for(var j = 0; j < sr_rulelist_col.length; j++){
					code +='<td width="'+wid[j]+'%">'+ sr_rulelist_col[j] +'</td>';		//IP  width="98"
				}
				code +='<td width="22%"><!--input class="edit_btn" onclick="edit_Row(this);" value=""/-->';
				code +='<input class="remove_btn" onclick="del_Row(this);" value=""/></td></tr>';
		}
	}
  code +='</table>';
	$("sr_rulelist_Block").innerHTML = code;
}
</script>
</head>

<body onload="initial();" onunLoad="return unload_body();">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="current_page" value="Advanced_GWStaticRoute_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_wait" value="10">
<input type="hidden" name="action_script" value="restart_net">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="sr_num_x_0" value="<% nvram_get("sr_num_x"); %>" readonly="1">
<input type="hidden" name="sr_rulelist" value=''>

<table class="content" align="center" cellpadding="0" cellspacing="0">
  <tr>
	<td width="17">&nbsp;</td>
	
	<!--=====Beginning of Main Menu=====-->
	<td valign="top" width="202">
	  <div id="mainMenu"></div>
	  <div id="subMenu"></div>
	</td>
	
    <td valign="top">
	<div id="tabMenu" class="submenuBlock"></div>
<!--===================================Beginning of Main Content===========================================-->
<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
  <tr>
	<td valign="top" >
	  <table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">
		<tbody>
		<tr>
		  <td bgcolor="#4D595D" valign="top"  >
		  <div>&nbsp;</div>
		  <div class="formfonttitle"><#menu5_2#> - <#menu5_2_3#></div>
		  <div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
      <div class="formfontdesc"><#RouterConfig_GWStaticEnable_sectiondesc#></div>
		  
			<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">				
			  <thead>
			  <tr>
				<td colspan="2"><#t2BC#></td>
			  </tr>
			  </thead>		

		  	<tr>
	    		<th><#RouterConfig_GWStaticEnable_itemname#></th>
	    		<td>
			  		<input type="radio" value="1" name="sr_enable_x" class="input" onclick="return change_common_radio(this, 'RouterConfig', 'sr_enable_x', '1')" <% nvram_match("sr_enable_x", "1", "checked"); %>><#checkbox_Yes#>
			  		<input type="radio" value="0" name="sr_enable_x" class="input" onclick="return change_common_radio(this, 'RouterConfig', 'sr_enable_x', '0')" <% nvram_match("sr_enable_x", "0", "checked"); %>><#checkbox_No#>
					</td>
		  	</tr>				
			</table>		
							
			<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table">
			<thead>
			  	<tr>
					<td colspan="6" id="GWStatic"><#RouterConfig_GWStatic_groupitemdesc#></td>
			  	</tr>
			</thead>			
				<tr>
					<!--th class="th_15_table"><a href="javascript:void(0);" onClick="openHint(6,1);"><div class="table_text"><#RouterConfig_GWStaticIP_itemname#></div></a></th>
					<th class="th_15_table"><a href="javascript:void(0);" onClick="openHint(6,2);"><div class="table_text"><#RouterConfig_GWStaticMask_itemname#></div></a></th>
					<th class="th_15_table"><a href="javascript:void(0);" onClick="openHint(6,3);"><div class="table_text"><#RouterConfig_GWStaticGW_itemname#></div></a></th>
					<th class="th_3_table"><a href="javascript:void(0);" onClick="openHint(6,4);"><div class="table_text"><#RouterConfig_GWStaticMT_itemname#></div></a></th>
					<th class="select_table"><a href="javascript:void(0);" onClick="openHint(6,5);"><div class="table_text"><#RouterConfig_GWStaticIF_itemname#></div></a></th>
					<th class="edit_table">Edit</th-->
					<th><a href="javascript:void(0);" onClick="openHint(6,1);"><div class="table_text"><#RouterConfig_GWStaticIP_itemname#></div></a></th>
					<th><a href="javascript:void(0);" onClick="openHint(6,2);"><div class="table_text"><#RouterConfig_GWStaticMask_itemname#></div></a></th>
					<th><a href="javascript:void(0);" onClick="openHint(6,3);"><div class="table_text"><#RouterConfig_GWStaticGW_itemname#></div></a></th>
					<th><a href="javascript:void(0);" onClick="openHint(6,4);"><div class="table_text"><#RouterConfig_GWStaticMT_itemname#></div></a></th>
					<th><a href="javascript:void(0);" onClick="openHint(6,5);"><div class="table_text"><#RouterConfig_GWStaticIF_itemname#></div></a></th>
					<th>Edit</th>					
			  </tr>
			  
			  <tr>
					<td width="20%"><input type="text" maxlength="15" class="input_15_table" name="sr_ipaddr_x_0" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)" onBlur="valid_IP_form(this,1)"></td>
					<td width="20%"><input type="text" maxlength="15" class="input_15_table" name="sr_netmask_x_0" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"></td>
					<td width="20%"><input type="text" maxlength="15" class="input_15_table" name="sr_gateway_x_0" onKeyPress="return is_ipaddr(this)" onKeyUp="change_ipaddr(this)"></td>
					<td width="8%"><input type="text" maxlength="3" class="input_3_table" name="sr_matric_x_0"  onKeyPress="return is_number_sp(event, this);"></td>
					<td width="10%">
						<select name="sr_if_x_o" class="input_option" style="width:62px;">
							<option  <% nvram_match_list_x("RouterConfig","sr_if_x", "LAN","selected", 0); %> value="LAN">LAN</option>
							<option  <% nvram_match_list_x("RouterConfig","sr_if_x", "MAN","selected", 0); %> value="MAN">MAN</option>
							<option  <% nvram_match_list_x("RouterConfig","sr_if_x", "WAN","selected", 0); %> value="WAN">WAN</option>
						</select>
					</td>
				
					<td width="22%">
						<div> 
							<input type="button" class="add_btn" onClick="addRow_Group(32);" value="">
						</div>
					</td>
			  </tr>
			  </table>		
			  
			  <div id="sr_rulelist_Block"></div>
			  	
				<div class="apply_gen">
					<input name="button" type="button" class="button_gen" onclick="applyRule();" value="<#CTL_apply#>"/>
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

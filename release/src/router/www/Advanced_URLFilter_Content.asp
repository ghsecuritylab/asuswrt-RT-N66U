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
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_5_2#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script type="text/javascript" language="JavaScript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/detect.js"></script>
<script>
wan_route_x = '<% nvram_get("wan_route_x"); %>';
wan_nat_x = '<% nvram_get("wan_nat_x"); %>';
wan_proto = '<% nvram_get("wan_proto"); %>';

<% login_state_hook(); %>
var wireless = [<% wl_auth_list(); %>];	// [[MAC, associated, authorized], ...]
var url_rulelist_array = '<% nvram_get("url_rulelist"); %>';

function initial(){
	show_menu();
	load_body();
	show_url_rulelist();
	enable_url();
	enable_url_1();
}


function show_url_rulelist(){
	var url_rulelist_row = url_rulelist_array.split('&#60');
	var code = "";
	code +='<table width="100%" border="1" cellspacing="0" cellpadding="4" align="center" class="list_table" id="url_rulelist_table">'; 
	if(url_rulelist_row.length == 1)
		code +='<tr><td style="color:#FFCC00;"><#IPConnection_VSList_Norule#></td>';
	else{
		for(var i =1; i < url_rulelist_row.length; i++){
		code +='<tr id="row'+i+'">';
		code +='<td width="80%">'+ url_rulelist_row[i] +'</td>';		//Url keyword
		code +='<td width="20%">';
		code +="<input class=\"remove_btn\" type=\"button\" onclick=\"deleteRow(this);\" value=\"\"/></td>";
		}
	}
  	code +='</tr></table>';
	
	$("url_rulelist_Block").innerHTML = code;
}

function deleteRow(r){
  var i=r.parentNode.parentNode.rowIndex;
  $('url_rulelist_table').deleteRow(i);
  
  var url_rulelist_value = "";
	for(i=0; i<$('url_rulelist_table').rows.length; i++){
		url_rulelist_value += "&#60";
		url_rulelist_value += $('url_rulelist_table').rows[i].cells[0].innerHTML;
	}
	
	url_rulelist_array =url_rulelist_value;
	if(url_rulelist_array == "")
		show_url_rulelist();
}

function addRow(obj, upper){
		var rule_num = $('url_rulelist_table').rows.length;
		var item_num = $('url_rulelist_table').rows[0].cells.length;
		
	if(rule_num >= upper){
		alert("<#JS_itemlimit1#> " + upper + " <#JS_itemlimit2#>");
		return false;	
	}	
	
	if(obj.value==""){
		alert("<#JS_fieldblank#>");
		obj.focus();
		obj.select();		
	
	}else{
		
		//Viz check same rule
		for(i=0; i<rule_num; i++){
			for(j=0; j<item_num-1; j++){		//only 1 value column
				if(obj.value == $('url_rulelist_table').rows[i].cells[j].innerHTML){
					alert("<#JS_duplicate#>");
					return false;
				}	
			}
		}
		
		
		
		url_rulelist_array += "&#60";
		url_rulelist_array += obj.value;
		obj.value = "";		
		show_url_rulelist();
	}	
}

function cross_midnight(){
	return changeDate();
}

function applyRule(){
	if(validForm()){
		var rule_num = $('url_rulelist_table').rows.length;
		var item_num = $('url_rulelist_table').rows[0].cells.length;
		var tmp_value = "";
	
		for(i=0; i<rule_num; i++){
			tmp_value += "<"		
			for(j=0; j<item_num-1; j++){	
				tmp_value += $('url_rulelist_table').rows[i].cells[j].innerHTML;
				if(j != item_num-2)	
					tmp_value += ">";
			}
		}
		if(tmp_value == "<"+"<#IPConnection_VSList_Norule#>" || tmp_value == "<")
			tmp_value = "";	
	
		document.form.url_rulelist.value = tmp_value;
		updateDateTime(document.form.current_page.value);

		if(document.form.url_enable_x.value != document.form.url_enable_x_orig.value)
			FormActions("start_apply.htm", "apply", "reboot", "30");	

		showLoading();
		document.form.submit();
	}
}

function validForm(){
		return true;
}

function enable_url(){
	if(document.form.url_enable_x[1].checked == 1)
		$("url_time").style.display = "none";
	else 
		$("url_time").style.display = "";	
	return change_common_radio(document.form.url_enable_x, 'FirewallConfig', 'url_enable_x', '1')
}

function enable_url_1(){
	if(document.form.url_enable_x_1[1].checked == 1)
		$("url_time_1").style.display = "none";
	else 
		$("url_time_1").style.display = "";	
	return change_common_radio(document.form.url_enable_x_1, 'FirewallConfig', 'url_enable_x_1', '1')
}

function done_validating(action){
	refreshpage();
}
</script>
</head>

<body onload="initial();" onunLoad="return unload_body();">
<div id="TopBanner"></div>

<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="current_page" value="Advanced_URLFilter_Content.asp">
<input type="hidden" name="next_page" value="Advanced_URLFilter_Content.asp">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_wait" value="5">
<input type="hidden" name="action_script" value="restart_firewall">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="url_date_x" value="<% nvram_get("url_date_x"); %>">
<input type="hidden" name="url_time_x" value="<% nvram_get("url_time_x"); %>">
<input type="hidden" name="url_time_x_1" value="<% nvram_get("url_time_x_1"); %>">
<input type="hidden" name="url_num_x_0" value="<% nvram_get("url_num_x"); %>" readonly="1">
<input type="hidden" name="url_rulelist" value="<% nvram_get("url_rulelist"); %>">
<input type="hidden" name="url_enable_x_orig" value="<% nvram_get("url_enable_x"); %>">
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
		  <div class="formfonttitle"><#menu5_5#> - <#menu5_5_2#></div>
		  <div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
		  <div class="formfontdesc"><#FirewallConfig_UrlFilterEnable_sectiondesc#></div>

			<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
					  <thead>
					  <tr>
						<td colspan="4"><#t2BC#></td>
					  </tr>
					  </thead>		
        <tr>
          <th><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(9,1);"><#FirewallConfig_UrlFilterEnable_itemname#></th>
         	<td>  
         		<input type="radio" value="1" name="url_enable_x" onClick="enable_url();" <% nvram_match("url_enable_x", "1", "checked"); %>><#CTL_Enabled#>
         		<input type="radio" value="0" name="url_enable_x" onClick="enable_url();" <% nvram_match("url_enable_x", "0", "checked"); %>><#CTL_Disabled#>
					</td>
        </tr>	
			</table>

<table width="100%" style="display:none;" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
         <tr>
          <th><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(9,1);"><#FirewallConfig_UrlFilterEnable_itemname#> 1:</th>
          	<td>  
          		<input type="radio" value="1" name="url_enable_x_0" onClick="enable_url();" <% nvram_match("url_enable_x", "1", "checked"); %>><#CTL_Enabled#>
          		<input type="radio" value="0" name="url_enable_x_0" onClick="enable_url();" <% nvram_match("url_enable_x", "0", "checked"); %>><#CTL_Disabled#>
		</td>
        </tr>	

	<tr id="url_time" style="display:none;">
          <th><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(9,2);"><#FirewallConfig_URLActiveTime_itemname#> 1:</a></th>
	        <td>
			<input type="text" maxlength="2" class="input_3_table" name="url_time_x_starthour" onKeyPress="return is_number_sp(event, this);" onblur="validate_timerange(this, 0);">:
			<input type="text" maxlength="2" class="input_3_table" name="url_time_x_startmin" onKeyPress="return is_number_sp(event, this);" onblur="validate_timerange(this, 1);">-
			<input type="text" maxlength="2" class="input_3_table" name="url_time_x_endhour" onKeyPress="return is_number_sp(event, this);" onblur="validate_timerange(this, 2);">:
			<input type="text" maxlength="2" class="input_3_table" name="url_time_x_endmin" onKeyPress="return is_number_sp(event, this);" onblur="validate_timerange(this, 3);">
		</td>
        </tr>
        
        <tr>
          <th><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(9,1);"><#FirewallConfig_UrlFilterEnable_itemname#> 2:</th>
          	<td>  
          		<input type="radio" value="1" name="url_enable_x_1" onClick="enable_url_1();" <% nvram_match("url_enable_x_1", "1", "checked"); %>><#CTL_Enabled#>
          		<input type="radio" value="0" name="url_enable_x_1" onClick="enable_url_1();" <% nvram_match("url_enable_x_1", "0", "checked"); %>><#CTL_Disabled#>
		</td>
        </tr>      
        
	<tr id="url_time_1" style="display:none;">
          <th><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(9,2);"><#FirewallConfig_URLActiveTime_itemname#> 2:</a></th>
          	<td>
			<input type="text" maxlength="2" class="input_3_table" name="url_time_x_starthour_1" onKeyPress="return is_number_sp(event, this);" onblur="validate_timerange(this, 0);">:
			<input type="text" maxlength="2" class="input_3_table" name="url_time_x_startmin_1" onKeyPress="return is_number_sp(event, this);" onblur="validate_timerange(this, 1);">-
			<input type="text" maxlength="2" class="input_3_table" name="url_time_x_endhour_1" onKeyPress="return is_number_sp(event, this);" onblur="validate_timerange(this, 2);">:
			<input type="text" maxlength="2" class="input_3_table" name="url_time_x_endmin_1" onKeyPress="return is_number_sp(event, this);" onblur="validate_timerange(this, 3);">
		</td>
        </tr>
        
        <tr>
          <th><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(9,1);"><#FirewallConfig_URLActiveDate_itemname#></a></th>
          <td>
		  	<input type="checkbox" name="url_date_x_Sun" class="input" onChange="return changeDate();">Sun
			<input type="checkbox" name="url_date_x_Mon" class="input" onChange="return changeDate();">Mon			
			<input type="checkbox" name="url_date_x_Tue" class="input" onChange="return changeDate();">Tue
			<input type="checkbox" name="url_date_x_Wed" class="input" onChange="return changeDate();">Wed
			<input type="checkbox" name="url_date_x_Thu" class="input" onChange="return changeDate();">Thu
			<input type="checkbox" name="url_date_x_Fri" class="input" onChange="return changeDate();">Fri
			<input type="checkbox" name="url_date_x_Sat" class="input" onChange="return changeDate();">Sat
		  </td>
        </tr>
        
        <!--tr>
          <th><a class="hintstyle"  href="javascript:void(0);" onClick="openHint(9,2);"><#FirewallConfig_URLActiveTime_itemname#></a></th>
          <td>
			<input type="text" maxlength="2" class="input" size="2" name="url_time_x_starthour" onKeyPress="return is_number(this)">:
			<input type="text" maxlength="2" class="input" size="2" name="url_time_x_startmin" onKeyPress="return is_number(this)">-
			<input type="text" maxlength="2" class="input" size="2" name="url_time_x_endhour" onKeyPress="return is_number(this)">:
			<input type="text" maxlength="2" class="input" size="2" name="url_time_x_endmin" onKeyPress="return is_number(this)">
		  </td>
        </tr-->
</table>

<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table">
					  <thead>
					  <tr>
						<td colspan="4"><#FirewallConfig_UrlList_groupitemdesc#></td>
					  </tr>
					  </thead>

  	<tr>
		<th width="80%"><#FirewallConfig_UrlList_groupitemdesc#></th>
		<th width="20%">Edit</th>
	</tr>
	<tr>
		<td width="80%">
	 		<input type="text" maxlength="32" class="input_32_table" name="url_keyword_x_0" onKeyPress="return is_string(this)">
	 	</td>
	 	<td width="20%">	
	  		<input class="add_btn" type="button" onClick="if(validForm()){addRow(document.form.url_keyword_x_0, 128);}" value="">
	 	</td>	
	</tr>
</table>
      	
      	<div id="url_rulelist_Block"></div>

		<div class="apply_gen">
			<input type="button" class="button_gen" onclick="applyRule()" value="<#CTL_apply#>"/>
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

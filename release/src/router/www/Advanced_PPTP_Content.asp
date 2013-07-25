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
<title>ASUS Wireless Router <#Web_Title#> - VPN Server</title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<script language="JavaScript" type="text/javascript" src="/state.js"></script>
<script language="JavaScript" type="text/javascript" src="/help.js"></script>
<script language="JavaScript" type="text/javascript" src="/general.js"></script>
<script language="JavaScript" type="text/javascript" src="/popup.js"></script>
<script language="JavaScript" type="text/javascript" src="/detect.js"></script>
<script>
wan_route_x = '<% nvram_get("wan_route_x"); %>';
wan_nat_x = '<% nvram_get("wan_nat_x"); %>';

<% login_state_hook(); %>
var wireless = [<% wl_auth_list(); %>];	// [[MAC, associated, authorized], ...]
var pptpd_clientlist_array = '<% nvram_get("pptpd_clientlist"); %>';

function initial(){
	show_menu();
	showpptpd_clientlist();
	if(document.form.pptpd_clients.value != "") {
		document.form._pptpd_clients_start.value = document.form.pptpd_clients.value.split(".")[3].split("-")[0];
		document.form._pptpd_clients_end.value = document.form.pptpd_clients.value.split(".")[3].split("-")[1];
	}
}

function applyRule(){
	var rule_num = $('pptpd_clientlist_table').rows.length;
	var item_num = $('pptpd_clientlist_table').rows[0].cells.length;
	var tmp_value = "";
	var pptpd_clients_start_ip = parseInt(document.form._pptpd_clients_start.value);
	var pptpd_clients_end_ip = parseInt(document.form._pptpd_clients_end.value);
	
	if(!valid_IP_form(document.form.pptpd_dns1, 0) || !valid_IP_form(document.form.pptpd_dns2, 0))
			return false;
	
	if(!valid_IP_form(document.form.pptpd_wins1, 0) || !valid_IP_form(document.form.pptpd_wins2, 0))
			return false;
	
	if(!validate_number_range(document.form._pptpd_clients_start, 2, 254)){
		  document.form._pptpd_clients_start.focus();
			return false;
	}		
	if(!validate_number_range(document.form._pptpd_clients_end, 2, 254)){
			document.form._pptpd_clients_end.focus();
			return false;
	}	
	
	if(pptpd_clients_start_ip > pptpd_clients_end_ip){
		alert("This value should be higher than "+document.form._pptpd_clients_start.value);
		document.form._pptpd_clients_end.focus();
		return false;
	}
  if( (pptpd_clients_end_ip - pptpd_clients_start_ip) > 9 ){
      alert("Pptpd server only allows max 10 clients!");
      document.form._pptpd_clients_end.focus();
      return false;
  }	
	
	for(i=0; i<rule_num; i++){
		tmp_value += "<"		
		for(j=0; j<item_num-1; j++){	
			tmp_value += $('pptpd_clientlist_table').rows[i].cells[j].childNodes[0].nodeValue;
			if(j != item_num-2)	
				tmp_value += ">";
		}
	}
	if(tmp_value == "<"+"<#IPConnection_VSList_Norule#>" || tmp_value == "<")
		tmp_value = "";	
	document.form.pptpd_clientlist.value = tmp_value;

	document.form.pptpd_clients.value = "192.168.10." + document.form._pptpd_clients_start.value + "-" + document.form._pptpd_clients_end.value;
	showLoading();
	document.form.submit();	
}

function addRow(obj, head){
	if(head == 1)
		pptpd_clientlist_array += "&#60"
	else
		pptpd_clientlist_array += "&#62"
			
	pptpd_clientlist_array += obj.value;

	obj.value = "";
}

function addRow_Group(upper){
	var rule_num = $('pptpd_clientlist_table').rows.length;
	var item_num = $('pptpd_clientlist_table').rows[0].cells.length;
		
	if(rule_num >= upper){
		alert("<#JS_itemlimit1#> " + upper + " <#JS_itemlimit2#>");
		return false;	
	}			
		
	if(document.form.pptpd_clientlist_username.value==""){
		alert("<#JS_fieldblank#>");
		document.form.pptpd_clientlist_username.focus();
		document.form.pptpd_clientlist_username.select();
		return false;
	}else if(document.form.pptpd_clientlist_password.value==""){
		alert("<#JS_fieldblank#>");
		document.form.pptpd_clientlist_password.focus();
		document.form.pptpd_clientlist_password.select();
		return false;
	}else{
		
		//Viz check same rule  //match(username) is not accepted
		if(item_num >=2){	
			for(i=0; i<rule_num; i++){	
					if(document.form.pptpd_clientlist_username.value.toLowerCase() == $('pptpd_clientlist_table').rows[i].cells[0].innerHTML.toLowerCase()){
						alert("<#JS_duplicate#>");
						document.form.pptpd_clientlist_username.focus();
						document.form.pptpd_clientlist_username.select();
						return false;
					}	
			}
		}		
		addRow(document.form.pptpd_clientlist_username ,1);
		addRow(document.form.pptpd_clientlist_password, 0);
		showpptpd_clientlist();		
	}
}

function del_Row(r){
  var i=r.parentNode.parentNode.rowIndex;
  $('pptpd_clientlist_table').deleteRow(i);
  
  var pptpd_clientlist_value = "";
	for(k=0; k<$('pptpd_clientlist_table').rows.length; k++){
		for(j=0; j<$('pptpd_clientlist_table').rows[k].cells.length-1; j++){
			if(j == 0)	
				pptpd_clientlist_value += "&#60";
			else
				pptpd_clientlist_value += "&#62";
			pptpd_clientlist_value += $('pptpd_clientlist_table').rows[k].cells[j].innerHTML;		
		}
	}
	
	pptpd_clientlist_array = pptpd_clientlist_value;
	if(pptpd_clientlist_array == "")
		showpptpd_clientlist();
}

function edit_Row(r){ 	
	var i=r.parentNode.parentNode.rowIndex;
	document.form.pptpd_clientlist_username.value = $('pptpd_clientlist_table').rows[i].cells[0].innerHTML;
	document.form.pptpd_clientlist_password.value = $('pptpd_clientlist_table').rows[i].cells[1].innerHTML; 	
  del_Row(r);	
}

function showpptpd_clientlist(){
	var pptpd_clientlist_row = pptpd_clientlist_array.split('&#60');
	var code = "";

	code +='<table width="100%" cellspacing="0" cellpadding="4" align="center" class="list_table" id="pptpd_clientlist_table">';
	if(pptpd_clientlist_row.length == 1)
		code +='<tr><td style="color:#FFCC00;" colspan="6"><#IPConnection_VSList_Norule#></td></tr>';
	else{
		for(var i = 1; i < pptpd_clientlist_row.length; i++){
			code +='<tr id="row'+i+'">';
			var pptpd_clientlist_col = pptpd_clientlist_row[i].split('&#62');
				for(var j = 0; j < pptpd_clientlist_col.length; j++){
					code +='<td width="40%">'+ pptpd_clientlist_col[j] +'</td>';		//IP  width="98"
				}
				code +='<td width="20%"><!--input class="edit_btn" onclick="edit_Row(this);" value=""/-->';
				code +='<input class="remove_btn" onclick="del_Row(this);" value=""/></td></tr>';
		}
	}
  code +='</table>';
	$("pptpd_clientlist_Block").innerHTML = code;
}
</script>
</head>

<body onload="initial();">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>
<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>
<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
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
			<input type="hidden" name="current_page" value="Advanced_PPTP_Content.asp">
			<input type="hidden" name="next_page" value="Advanced_PPTP_Content.asp">
			<input type="hidden" name="next_host" value="">
			<input type="hidden" name="modified" value="0">
			<input type="hidden" name="action_mode" value="apply">
			<input type="hidden" name="action_wait" value="5">
			<input type="hidden" name="action_script" value="restart_pptpd">
			<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
			<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
			<input type="hidden" name="wl_ssid" value="<% nvram_get("wl_ssid"); %>">
			<input type="hidden" name="pptpd_clientlist" value="<% nvram_get("pptpd_clientlist"); %>">
			<input type="hidden" name="pptpd_clients" value="<% nvram_get("pptpd_clients"); %>">

			<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
				<tr>
					<td valign="top" >
						<table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">
							<tbody>
								<tr>
								  <td bgcolor="#4D595D" valign="top">
								  <div>&nbsp;</div>
								  <div class="formfonttitle">VPN Server</div>
								  <div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
								  <div class="formfontdesc"><#PPTP_desc#></div>
								  <div class="formfontdesc"><#PPTP_desc2#> <% nvram_get("wan0_ipaddr"); %></div>
								  <div class="formfontdesc" style="margin-top:-10px;"><#PPTP_desc3#></div>
									<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3"  class="FormTable">
									  	<thead>
									  		<tr>
												<td colspan="3" id="GWStatic"><#vpn_setting#></td>
									  		</tr>
									  	</thead>
										
										<tr>
											<th><#vpn_enable#></th>
											<td>
												<select name="pptpd_enable" class="input_option">
													<option class="content_input_fd" value="0" <% nvram_match("pptpd_enable", "0","selected"); %>><#btn_disable#></option>
													<option class="content_input_fd" value="1"<% nvram_match("pptpd_enable", "1","selected"); %>><#btn_Enable#></option>
												</select>			
											</td>
									  </tr>
							
										<tr>
											<th><#vpn_broadcast#></th>
											<td>
												<select name="pptpd_broadcast" class="input_option">
													<option class="content_input_fd" value="disable" <% nvram_match("pptpd_broadcast", "0","selected"); %>><#btn_disable#></option>
													<option class="content_input_fd" value="br0"<% nvram_match("pptpd_broadcast", "br0","selected"); %>>LAN to VPN Client</option>
                                                                                                        <option class="content_input_fd" value="ppp" <% nvram_match("pptpd_broadcast", "ppp","selected"); %>>VPN Client to LAN</option>
                                                                                                        <option class="content_input_fd" value="br0ppp"<% nvram_match("pptpd_broadcast", "br0ppp","selected"); %>>Both</option>
												</select>			
											</td>
									  </tr>
							
										<tr>
											<th>Force MPPE Encryption</th>
											<td>
												<select name="pptpd_forcemppe" class="input_option">
													<option value="auto" <% nvram_match("pptpd_forcemppe", "auto","selected"); %>>Auto</option>
													<option value="none" <% nvram_match("pptpd_forcemppe", "none","selected"); %>>No Encryption</option>
													<option value="+mppe-40" <% nvram_match("pptpd_forcemppe", "+mppe-40","selected"); %>>MPPE 40</option>
													<option value="+mppe-128" <% nvram_match("pptpd_forcemppe", "+mppe-128","selected"); %>>MPPE 128</option> 

												</select>			
											</td>
									  </tr>

			          		<tr>
			            		<th><a class="hintstyle" href="javascript:void(0);"><#IPConnection_x_DNSServer1_itemname#></a></th>
			            		<td><input type="text" maxlength="15" class="input_15_table" name="pptpd_dns1" value="<% nvram_get("pptpd_dns1"); %>" onkeypress="return is_ipaddr(this)" onkeyup="change_ipaddr(this)"/></td>
			          		</tr>

			          		<tr>
			            		<th><a class="hintstyle" href="javascript:void(0);"><#IPConnection_x_DNSServer2_itemname#></a></th>
			            		<td><input type="text" maxlength="15" class="input_15_table" name="pptpd_dns2" value="<% nvram_get("pptpd_dns2"); %>" onkeypress="return is_ipaddr(this)" onkeyup="change_ipaddr(this)"/></td>
			          		</tr>

			          		<tr>
			            		<th>WINS 1</th>
			            		<td><input type="text" maxlength="15" class="input_15_table" name="pptpd_wins1" value="<% nvram_get("pptpd_wins1"); %>" onkeypress="return is_ipaddr(this)" onkeyup="change_ipaddr(this)"/></td>
			          		</tr>

			          		<tr>
			            		<th>WINS 2</th>
			            		<td><input type="text" maxlength="15" class="input_15_table" name="pptpd_wins2" value="<% nvram_get("pptpd_wins2"); %>" onkeypress="return is_ipaddr(this)" onkeyup="change_ipaddr(this)"/></td>
			          		</tr>

			          		<tr>
			            		<th><#vpn_client_ip#></th>
			            		<td>
							 <span style="font-family: Lucida Console;color: #FFF;">192.168.10.</span><input type="text" maxlength="3" class="input_3_table" name="_pptpd_clients_start" value=""/> ~
                                                         <span style="font-family: Lucida Console;color: #FFF;">192.168.10.</span><input type="text" maxlength="3" class="input_3_table" name="_pptpd_clients_end" value=""/><span style="color:#FFCC00;"> <#vpn_maximum_clients#></span>
						</td>
			          		</tr>

									</table>

									<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table" style="margin-top:8px;">
									  	<thead>
									  		<tr>
												<td colspan="3" id="GWStatic">Username and Password</td>
									  		</tr>
									  	</thead>
									  
									  	<tr>
								  			<th><#PPPConnection_UserName_itemname#></th>
						        		<th><#PPPConnection_Password_itemname#></th>
						        		<th>Edit</th>
									  	</tr>			  
									  	<tr>
						          	<td width="40%">
						              <input type="text" class="input_25_table" maxlength="64" name="pptpd_clientlist_username" >
						            </td>
						            			<td width="40%">
						            				<input type="text" class="input_25_table" maxlength="64" name="pptpd_clientlist_password">
						            			</td>
						            			<td width="20%">
																<div> 
																	<input type="button" class="add_btn" onClick="addRow_Group(32);" value="">
																</div>
						            			</td>
									  	</tr>	 			  
								  </table>        			
						        			
				  <div id="pptpd_clientlist_Block"></div>
        			
        	<!-- manually assigned the DHCP List end-->		
					<div class="apply_gen">
						<input class="button_gen" onclick="applyRule()" type="button" value="<#CTL_apply#>"/>
					</div>		
      	  </td>
		</tr>

							</tbody>
						</table>
					</td>
				</tr>
			</table>
    <td width="10" align="center" valign="top">&nbsp;</td>
	</tr>
</table>
</form>
<div id="footer"></div>
</body>
</html>

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
<title>ASUS Wireless Router <#Web_Title#> - <#menu5_3_2#></title>
<link rel="stylesheet" type="text/css" href="/index_style.css"> 
<link rel="stylesheet" type="text/css" href="/form_style.css">

<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/general.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/detect.js"></script>

<style>
.Portrange{
	font-size: 12px;
	font-family: Lucida Console;
}
</style>
<script>
wan_route_x = '<% nvram_get("wan_route_x"); %>';
wan_nat_x = '<% nvram_get("wan_nat_x"); %>';
wan_proto = '<% nvram_get("wan_proto"); %>';

<% login_state_hook(); %>
var wireless = [<% wl_auth_list(); %>];	// [[MAC, associated, authorized], ...]

var qos_rulelist_array = '<% nvram_get("qos_rulelist"); %>';
var qos_orates = '<% nvram_get("qos_orates"); %>';
var qos_irates = '<% nvram_get("qos_irates"); %>';

var overlib_str0 = new Array();	//Viz add 2011.06 for record longer qos rule desc
var overlib_str = new Array();	//Viz add 2011.06 for record longer portrange value

function initial(){
	show_menu();
	showqos_rulelist();
	//bw_crtl_display("packet_table_display_id", "packet_table"); //for packet_table
	//bw_crtl_display("bw_crtl_display_id", "bandwidth_level"); //for bandwidth_level
	gen_options();
	load_QoS_rule();
}

function get_obj_id(id){
	return document.getElementById(id);
}

function bw_crtl_display(div_ctl, div_table){
	var obj_id = get_obj_id(div_ctl);
	
	if(obj_id.innerHTML == "+"){
		obj_id.innerHTML = "-";
		gen_options();
		blocking(div_table, true);
	}
	else{
		obj_id.innerHTML = "+";
		blocking(div_table ,false);
	}
}

function applyRule(){

	if(!validate_range(document.form.qos_obw, 1, 9999999999)){
		return false;
	}else if(!validate_range(document.form.qos_ibw, 1, 9999999999)){
		return false;
  }
	
	if(save_options() != false){
		save_table();
		showLoading();
	 	save_checkbox();
		document.form.submit();
	}
}

function save_checkbox(){
	document.form.qos_ack.value = document.form.qos_ack_checkbox.checked ? "on" : "off";
	document.form.qos_syn.value = document.form.qos_syn_checkbox.checked ? "on" : "off";
	document.form.qos_fin.value = document.form.qos_fin_checkbox.checked ? "on" : "off";
	document.form.qos_rst.value = document.form.qos_rst_checkbox.checked ? "on" : "off";
	document.form.qos_icmp.value = document.form.qos_icmp_checkbox.checked ? "on" : "off";
}

function save_table(){
	var rule_num = $('qos_rulelist_table').rows.length;
	var item_num = $('qos_rulelist_table').rows[0].cells.length;
	var tmp_value = "";
     var comp_tmp = "";

	for(i=0; i<rule_num; i++){
		tmp_value += "<"		
		for(j=0; j<item_num-1; j++){	
						
			if(j==5){
					if($('qos_rulelist_table').rows[i].cells[j].innerHTML=="<#Highest#>")
						tmp_value += 0;
					else if($('qos_rulelist_table').rows[i].cells[j].innerHTML=="<#High#>")
						tmp_value += 1;
					else if($('qos_rulelist_table').rows[i].cells[j].innerHTML=="<#Medium#>")
						tmp_value += 2;
					else if($('qos_rulelist_table').rows[i].cells[j].innerHTML=="<#Low#>")
						tmp_value += 3;
					else
						tmp_value += 4;
				
			}else{
						
			if($('qos_rulelist_table').rows[i].cells[j].innerHTML.lastIndexOf("...")<0){
				tmp_value += $('qos_rulelist_table').rows[i].cells[j].innerHTML;
			}else{
				tmp_value += $('qos_rulelist_table').rows[i].cells[j].title;
			}
			}
			
			if(j != item_num-2)	
				tmp_value += ">";
		}
	}
	if(tmp_value == "<"+"<#IPConnection_VSList_Norule#>" || tmp_value == "<")
		tmp_value = "";	
	document.form.qos_rulelist.value = tmp_value;
}

function save_options(){
	document.form.qos_orates.value = "";
	for(var j=0; j<5; j++){
		var upload_bw_max = eval("document.form.upload_bw_max_"+j);
		var upload_bw_min = eval("document.form.upload_bw_min_"+j);

		var download_bw_min = eval("document.form.download_bw_min_"+j);

		if(parseInt(upload_bw_max.value) < parseInt(upload_bw_min.value)){
			alert("Invalid bandwidth period!");
			upload_bw_max.focus();
			return false;
		}

		document.form.qos_orates.value += upload_bw_min.value + "-" + upload_bw_max.value + ",";
		document.form.qos_irates.value += download_bw_min.value + ",";
	}
	document.form.qos_orates.value += "0-0,0-0,0-0,0-0,0-0";
	document.form.qos_irates.value += "0,0,0,0,0";
	return true;
}

function done_validating(action){
	refreshpage();
}

function addRow(obj, head){
	if(head == 1)
		qos_rulelist_array += "&#60"
	else
		qos_rulelist_array += "&#62"
			
	qos_rulelist_array += obj.value;
	obj.value= "";
	document.form.qos_min_transfered_x_0.value= "";
	document.form.qos_max_transfered_x_0.value= "";
}

function addRow_Group(upper){
	var rule_num = $('qos_rulelist_table').rows.length;
	var item_num = $('qos_rulelist_table').rows[0].cells.length;	

	if(rule_num >= upper){
		alert("<#JS_itemlimit1#> " + upper + " <#JS_itemlimit2#>");
		return false;	
	}	
	
	if(document.form.qos_service_name_x_0.value.indexOf("<")>=0
		|| document.form.qos_service_name_x_0.value.indexOf(">")>=0){
		alert("<#JS_validchar#>");
		document.form.qos_service_name_x_0.focus();
		document.form.qos_service_name_x_0.select();
		return false;
	}
	
	replace_symbol();
	if(document.form.qos_port_x_0.value != "" && !Check_multi_range(document.form.qos_port_x_0, 1, 65535)){
		parse_port="";
		return false;
	}	
		
	conv_to_transf();	
	if(item_num >=2){
			for(i=0; i<rule_num; i++){	
				if(overlib_str[i]){

					if(document.form.qos_ip_x_0.value == $('qos_rulelist_table').rows[i].cells[1].innerHTML 
						&& document.form.qos_port_x_0.value == overlib_str[i]
						&& document.form.qos_proto_x_0.value == $('qos_rulelist_table').rows[i].cells[3].innerHTML 
						&& document.form.qos_transfered_x_0.value == $('qos_rulelist_table').rows[i].cells[4].innerHTML){
							alert("<#JS_duplicate#>");
							parse_port="";
							document.form.qos_port_x_0.value =="";
							document.form.qos_ip_x_0.focus();
							document.form.qos_ip_x_0.select();
							return false;
					}
				}else{
					if(document.form.qos_ip_x_0.value == $('qos_rulelist_table').rows[i].cells[1].innerHTML 
						&& document.form.qos_port_x_0.value == $('qos_rulelist_table').rows[i].cells[2].innerHTML
						&& document.form.qos_proto_x_0.value == $('qos_rulelist_table').rows[i].cells[3].innerHTML 
						&& document.form.qos_transfered_x_0.value == $('qos_rulelist_table').rows[i].cells[4].innerHTML){
							alert("<#JS_duplicate#>");							
							parse_port="";
							document.form.qos_port_x_0.value =="";
							document.form.qos_ip_x_0.focus();
							document.form.qos_ip_x_0.select();
							return false;
					}										
				}	
			}
		}
	
	
	addRow(document.form.qos_service_name_x_0 ,1);
	addRow(document.form.qos_ip_x_0, 0);
	addRow(document.form.qos_port_x_0, 0);
	addRow(document.form.qos_proto_x_0, 0);
	document.form.qos_proto_x_0.value="tcp/udp";
	if(document.form.qos_transfered_x_0.value == "~")
		document.form.qos_transfered_x_0.value = "";
	addRow(document.form.qos_transfered_x_0, 0);
	addRow(document.form.qos_prio_x_0, 0);
	document.form.qos_prio_x_0.value="High";
	showqos_rulelist();
}

function del_Row(r){
  var i=r.parentNode.parentNode.rowIndex;
  $('qos_rulelist_table').deleteRow(i);
  
  var qos_rulelist_value = "";
	for(k=0; k<$('qos_rulelist_table').rows.length; k++){
		for(j=0; j<$('qos_rulelist_table').rows[k].cells.length-1; j++){
			if(j == 0)	
				qos_rulelist_value += "&#60";
			else
				qos_rulelist_value += "&#62";
				
			if($('qos_rulelist_table').rows[k].cells[j].innerHTML.lastIndexOf("...")<0){	
				qos_rulelist_value += $('qos_rulelist_table').rows[k].cells[j].innerHTML;	
			}else{
				qos_rulelist_value += $('qos_rulelist_table').rows[k].cells[j].title;
			}		
		}
	}
	
	qos_rulelist_array = qos_rulelist_value;
	if(qos_rulelist_array == "")
		showqos_rulelist();
}

var qos_rulelist_row;
function showqos_rulelist(){
	qos_rulelist_row = qos_rulelist_array.split('&#60');
	var code = "";

	code +='<table width="100%"  border="1" align="center" cellpadding="4" cellspacing="0" class="list_table" id="qos_rulelist_table">';
	if(qos_rulelist_row.length == 1)	// no exist "<"
		code +='<tr><td style="color:#FFCC00;" colspan="10"><#IPConnection_VSList_Norule#></td></tr>';
	else{
		for(var i = 1; i < qos_rulelist_row.length; i++){
			overlib_str0[i] ="";
			overlib_str[i] ="";			
			code +='<tr id="row'+i+'">';
			var qos_rulelist_col = qos_rulelist_row[i].split('&#62');
			var wid=[18, 17, 15, 13, 16, 13];						
				for(var j = 0; j < qos_rulelist_col.length; j++){
						if(j != 0 && j !=2 && j!=5){
							code +='<td width="'+wid[j]+'%">'+ qos_rulelist_col[j] +'</td>';
						}else if(j==0){
							if(qos_rulelist_col[0].length >15){
								overlib_str0[i] += qos_rulelist_col[0];
								qos_rulelist_col[0]=qos_rulelist_col[0].substring(0, 13)+"...";
								code +='<td width="'+wid[j]+'%"  title="'+overlib_str0[i]+'">'+ qos_rulelist_col[0] +'</td>';
							}else
								code +='<td width="'+wid[j]+'%">'+ qos_rulelist_col[j] +'</td>';
						}else if(j==2){
							if(qos_rulelist_col[2].length >13){
								overlib_str[i] += qos_rulelist_col[2];
								qos_rulelist_col[2]=qos_rulelist_col[2].substring(0, 11)+"...";
								code +='<td width="'+wid[j]+'%"  title="'+overlib_str[i]+'">'+ qos_rulelist_col[2] +'</td>';
							}else
								code +='<td width="'+wid[j]+'%">'+ qos_rulelist_col[j] +'</td>';																						
						}else if(j==5){
								if(qos_rulelist_col[5] =="0"){
										code +='<td width="'+wid[j]+'%"><#Highest#></td>';
								}else if(qos_rulelist_col[5] =="1"){
										code +='<td width="'+wid[j]+'%"><#High#></td>';
								}else if(qos_rulelist_col[5] =="2"){
										code +='<td width="'+wid[j]+'%"><#Medium#></td>';
								}else if(qos_rulelist_col[5] =="3"){
										code +='<td width="'+wid[j]+'%"><#Low#></td>';
								}else{
										code +='<td width="'+wid[j]+'%"><#Lowest#></td>';
								}	 		
						}else{
						}
				}
				code +='<td  width="8%"><!--input class="edit_btn" type="button" onclick="edit_Row(this);"/-->';
				code +='<input class="remove_btn" type="button" onclick="del_Row(this);"/></td></tr>';
		}
	}
	code +='</table>';
	$("qos_rulelist_Block").innerHTML = code;
	
	
	parse_port="";
}

function edit_Row(r){ 	
	var i=r.parentNode.parentNode.rowIndex;
	var qos_transfered_x_0_value = $('qos_rulelist_table').rows[i].cells[4].innerHTML.split('~');
	document.form.qos_service_name_x_0.value = $('qos_rulelist_table').rows[i].cells[0].innerHTML;
	document.form.qos_ip_x_0.value = $('qos_rulelist_table').rows[i].cells[1].innerHTML; 
	document.form.qos_port_x_0.value = $('qos_rulelist_table').rows[i].cells[2].innerHTML; 
	document.form.qos_proto_x_0.value = $('qos_rulelist_table').rows[i].cells[3].innerHTML;
	document.form.qos_min_transfered_x_0.value = qos_transfered_x_0_value[0];
	document.form.qos_max_transfered_x_0.value = qos_transfered_x_0_value[1];
	document.form.qos_prio_x_0.value = $('qos_rulelist_table').rows[i].cells[5].innerHTML;
  del_Row(r);	
}

function conv_to_transf(){
	if(document.form.qos_min_transfered_x_0.value =="" &&document.form.qos_max_transfered_x_0.value =="")
		document.form.qos_transfered_x_0.value = "";
	else	
		document.form.qos_transfered_x_0.value = document.form.qos_min_transfered_x_0.value + "~" + document.form.qos_max_transfered_x_0.value;
}

function gen_options(){
	if($("upload_bw_min_0").innerHTML == ""){
		var qos_orates_row = qos_orates.split(',');
		var qos_irates_row = qos_irates.split(',');
		for(var j=0; j<5; j++){
			var upload_bw_max = eval("document.form.upload_bw_max_"+j);
			var upload_bw_min = eval("document.form.upload_bw_min_"+j);
			//Viz 2011.06 var download_bw_max = eval("document.form.download_bw_max_"+j);
			var download_bw_min = eval("document.form.download_bw_min_"+j);
			var qos_orates_col = qos_orates_row[j].split('-');
			var qos_irates_col = qos_irates_row[j].split('-');
			for(var i=0; i<101; i++){
				add_options_value(upload_bw_min, i, qos_orates_col[0]);
				add_options_value(upload_bw_max, i, qos_orates_col[1]);
				add_options_value(download_bw_min, i, qos_irates_col[0]);
				//Viz 2011.06 add_options_value(download_bw_max, i, qos_irates_col[1]);
			}
			var upload_bw_desc = eval('document.getElementById("upload_bw_'+j+'_desc")');
			var download_bw_desc = eval('document.getElementById("download_bw_'+j+'_desc")');	

			upload_bw_desc.innerHTML = Math.round(upload_bw_min.value*document.form.qos_obw.value/100) + " ~ " + Math.round(upload_bw_max.value*document.form.qos_obw.value/100) + " kb/s";
			download_bw_desc.innerHTML = "0 ~ " + Math.round(download_bw_min.value*document.form.qos_ibw.value/100) +" kb/s";
			//Viz 2011.06 download_bw_desc.innerHTML = download_bw_min.value*document.form.qos_ibw.value/100 + " ~ " + download_bw_max.value*document.form.qos_ibw.value/100 + " kb/s";
		}
	}
	else{
		for(var j=0; j<5; j++){
			var upload_bw_max = eval("document.form.upload_bw_max_"+j);
			var upload_bw_min = eval("document.form.upload_bw_min_"+j);
			var download_bw_min = eval("document.form.download_bw_min_"+j);
			var upload_bw_desc = eval('document.getElementById("upload_bw_'+j+'_desc")');
			var download_bw_desc = eval('document.getElementById("download_bw_'+j+'_desc")');	
			upload_bw_desc.innerHTML = Math.round(upload_bw_min.value*document.form.qos_obw.value/100) + " ~ " + Math.round(upload_bw_max.value*document.form.qos_obw.value/100) + " kb/s";
			download_bw_desc.innerHTML = "0 ~ " + Math.round(download_bw_min.value*document.form.qos_ibw.value/100) +" kb/s";
		}
	}
}

function add_options_value(o, arr, orig){
	if(orig == arr)
		add_option(o, arr, arr, 1);
	else
		add_option(o, arr, arr, 0);
}

function switchPage(page){
	if(page == "2")
		return false;
	else
		location.href = "/QoS_EZQoS.asp";
}




function validate_multi_port(val, min, max){
	for(i=0; i<val.length; i++)
	{
		if (val.charAt(i)<'0' || val.charAt(i)>'9')
		{
			alert('<#JS_validrange#> ' + min + ' <#JS_validrange_to#> ' + max);
			return false;
		}
		if(val<min || val>max) {
			alert('<#JS_validrange#> ' + min + ' <#JS_validrange_to#> ' + max);
			return false;
		}else{
			val = str2val(val);
			if(val=="")
				val="0";
			return true;
		}				
	}	
}
function validate_multi_range(val, mini, maxi){
	var rangere=new RegExp("^([0-9]{1,5})\:([0-9]{1,5})$", "gi");
	if(rangere.test(val)){
		
		if(!validate_each_port(document.form.qos_port_x_0, RegExp.$1, mini, maxi) || !validate_each_port(document.form.qos_port_x_0, RegExp.$2, mini, maxi)){
				return false;								
		}else if(parseInt(RegExp.$1) >= parseInt(RegExp.$2)){
				alert("<#JS_validport#>");	
				return false;												
		}else				
			return true;	
	}else{
		if(!validate_single_range(val, mini, maxi)){	
					return false;											
				}
				return true;								
			}	
	}

function validate_single_range(val, min, max) {
	for(j=0; j<val.length; j++){		//is_number
		if (val.charAt(j)<'0' || val.charAt(j)>'9'){			
			alert('<#JS_validrange#> ' + min + ' <#JS_validrange_to#> ' + max);
			return false;
		}
	}
	
	if(val < min || val > max) {		//is_in_range		
		alert('<#JS_validrange#> ' + min + ' <#JS_validrange_to#> ' + max);
		return false;
	}else	
		return true;
	}		

var parse_port="";
function Check_multi_range(obj, mini, maxi){	
	obj.value = document.form.qos_port_x_0.value.replace(/[-~]/gi,":");	// "~-" to ":"
	var PortSplit = obj.value.split(",");			
	for(i=0;i<PortSplit.length;i++){
		PortSplit[i] = PortSplit[i].replace(/(^\s*)|(\s*$)/g, ""); 		// "\space" to ""
		PortSplit[i] = PortSplit[i].replace(/(^0*)/g, ""); 		// "^0" to ""	
				
		/*if(PortSplit[i] == "" ||PortSplit[i] == 0){
			alert("<#JS_ipblank1#>");
			obj.focus();
			obj.select();			
			return false;
		}*/
		if(!validate_multi_range(PortSplit[i], mini, maxi)){
			obj.focus();
			obj.select();
			return false;
		}						
		
		if(i ==PortSplit.length -1)
			parse_port = parse_port + PortSplit[i];
		else
			parse_port = parse_port + PortSplit[i] + ",";
			
	}
	document.form.qos_port_x_0.value = parse_port;
	return true;	
}

// Viz add 2011.06 Load default qos rule from XML
var url_link = "/ajax_qos_default.asp";
function load_QoS_rule(){		
	free_options(document.form.qos_default_sel);
	add_option(document.form.qos_default_sel, "<#Select_menu_default#>", 0, 1);
	
	loadXMLDoc(url_link);
		
}


var xmlhttp;
function loadXMLDoc(url_link){
	var ie = window.ActiveXObject;
	if (ie){		//IE
		xmlhttp = new ActiveXObject("Microsoft.XMLDOM");
		xmlhttp.async = false;
  		if (xmlhttp.readyState==4){
			xmlhttp.load(url_link);
			Load_XML2form();
		}						
		
	}else{	// FF Chrome Safari...
  		xmlhttp=new XMLHttpRequest();
  		if (xmlhttp.overrideMimeType){
			xmlhttp.overrideMimeType('text/xml');
		}
			
		xmlhttp.onreadystatechange = alertContents_qos;	
		xmlhttp.open("GET",url_link,true);
		xmlhttp.send();		
	}					
}

function alertContents_qos()
{
	if (xmlhttp != null && xmlhttp.readyState != null && xmlhttp.readyState == 4){
		if (xmlhttp.status != null && xmlhttp.status == 200){
				Load_XML2form();
		}
	}
}

// Load XML to Select option & save all info
var QoS_rules;
var Sel_desc, Sel_port, Sel_proto, Sel_rate, Sel_prio;
var rule_desc = new Array();
var rule_port = new Array();
var rule_proto = new Array();
var rule_rate = new Array();
var rule_prio = new Array();
function Load_XML2form(){			
			
			if (window.ActiveXObject){		//IE			
				QoS_rules = xmlhttp.getElementsByTagName("qos_rule");
			}else{	// FF Chrome Safari...
				//QoS_rules = xmlhttp.responseXML.documentElement.getElementsByTagName("qos_rule");
				QoS_rules = xmlhttp.responseXML.getElementsByTagName("qos_rule");
			}
				
			for(i=0;i<QoS_rules.length;i++){
				Sel_desc=QoS_rules[i].getElementsByTagName("desc");
				Sel_port=QoS_rules[i].getElementsByTagName("port");
				Sel_proto=QoS_rules[i].getElementsByTagName("proto");
				Sel_rate=QoS_rules[i].getElementsByTagName("rate");
				Sel_prio=QoS_rules[i].getElementsByTagName("prio");
								
				add_option(document.form.qos_default_sel, Sel_desc[0].firstChild.nodeValue, i, 0);
				//document.form.qos_default_sel.onchange = gen_options();

				if(Sel_desc[0].firstChild != null)
					rule_desc[i] = Sel_desc[0].firstChild.nodeValue;
				else
					rule_desc[i] ="";
				
				if(Sel_port[0].firstChild != null)						
					rule_port[i] = Sel_port[0].firstChild.nodeValue;
				else
					rule_port[i] ="";
				
				if(Sel_proto[0].firstChild != null)
					rule_proto[i] = Sel_proto[0].firstChild.nodeValue;
				else
					rule_proto[i] ="";	
				
				if(Sel_rate[0].firstChild != null)
					rule_rate[i] = Sel_rate[0].firstChild.nodeValue;
				else
					rule_rate[i] ="";	
					
				if(Sel_prio[0].firstChild != null)
					rule_prio[i] = Sel_prio[0].firstChild.nodeValue;
				else
					rule_prio[i] ="";	
				
				//alert(rule_desc[i]+','+rule_port[i]+','+rule_proto[i]+','+rule_rate[i]+','+rule_prio[i]);
			}	
	showQoSList();
}

//Viz add 2011.06 change qos_sel
function change_wizard(obj){
		for(var j = 0; j < QoS_rules.length; j++){
			if(rule_desc[j] != null && obj.value == j){

				if(rule_proto[j] == "TCP")
					document.form.qos_proto_x_0.options[0].selected = 1;
				else if(rule_proto[j] == "UDP")
					document.form.qos_proto_x_0.options[1].selected = 1;
				else if(rule_proto[j] == "TCP/UDP")
					document.form.qos_proto_x_0.options[2].selected = 1;
				else if(rule_proto[j] == "ANY")
					document.form.qos_proto_x_0.options[3].selected = 1;
				/*	marked By Viz 2011.12 for "iptables -p"
				else if(rule_proto[j] == "ICMP")
					document.form.qos_proto_x_0.options[3].selected = 1;
				else if(rule_proto[j] == "IGMP")
					document.form.qos_proto_x_0.options[4].selected = 1;	*/
				else
					document.form.qos_proto_x_0.options[0].selected = 1;
				
				if(rule_prio[j] == "<#Highest#>")
					document.form.qos_prio_x_0.options[0].selected = 1;
				else if(rule_prio[j] == "<#High#>")
					document.form.qos_prio_x_0.options[1].selected = 1;
				else if(rule_prio[j] == "<#Medium#>")
					document.form.qos_prio_x_0.options[2].selected = 1;
				else if(rule_prio[j] == "<#Low#>")
					document.form.qos_prio_x_0.options[3].selected = 1;
				else if(rule_prio[j] == "<#Lowest#>")
					document.form.qos_prio_x_0.options[4].selected = 1;	
				else
					document.form.qos_prio_x_0.options[2].selected = 1;
				
				if(rule_rate[j] == ""){
					document.form.qos_min_transfered_x_0.value = "";
					document.form.qos_max_transfered_x_0.value = "";
				}else{
					var trans=rule_rate[j].split("~");
					document.form.qos_min_transfered_x_0.value = trans[0];
					document.form.qos_max_transfered_x_0.value = trans[1];
				}	
				
				
				document.form.qos_service_name_x_0.value = rule_desc[j];
				document.form.qos_port_x_0.value = rule_port[j];
				//document.form.qos_transfered_x_0.value = rule_rate[j];
				break;
			}
		}
}

/*------------ Mouse event of fake LAN IP select menu {-----------------*/
function setClientIP(j){
	document.form.qos_service_name_x_0.value = rule_desc[j];

	if(rule_rate[j] == ""){
		document.form.qos_min_transfered_x_0.value = "";
		document.form.qos_max_transfered_x_0.value = "";
	}
	else{
		var trans=rule_rate[j].split("~");
		document.form.qos_min_transfered_x_0.value = trans[0];
		document.form.qos_max_transfered_x_0.value = trans[1];
	}	
	
	if(rule_proto[j] == "TCP")
		document.form.qos_proto_x_0.options[0].selected = 1;
	else if(rule_proto[j] == "UDP")
		document.form.qos_proto_x_0.options[1].selected = 1;
	else if(rule_proto[j] == "TCP/UDP")
		document.form.qos_proto_x_0.options[2].selected = 1;
	else if(rule_proto[j] == "ANY")
		document.form.qos_proto_x_0.options[3].selected = 1;
	/* marked By Viz 2011.12 for "iptables -p"
	else if(rule_proto[j] == "ICMP")
		document.form.qos_proto_x_0.options[3].selected = 1;
	else if(rule_proto[j] == "IGMP")
		document.form.qos_proto_x_0.options[4].selected = 1;	*/	
	else
		document.form.qos_proto_x_0.options[0].selected = 1;

	if(rule_prio[j] == "Highest")
		document.form.qos_prio_x_0.options[0].selected = 1;
	else if(rule_prio[j] == "High")
		document.form.qos_prio_x_0.options[1].selected = 1;
	else if(rule_prio[j] == "Medium")
		document.form.qos_prio_x_0.options[2].selected = 1;
	else if(rule_prio[j] == "Low")
		document.form.qos_prio_x_0.options[3].selected = 1;
	else if(rule_prio[j] == "Lowest")
		document.form.qos_prio_x_0.options[4].selected = 1;	
	else
		document.form.qos_prio_x_0.options[2].selected = 1;

	document.form.qos_service_name_x_0.value = rule_desc[j];
	document.form.qos_port_x_0.value = rule_port[j];

	hideClients_Block();
	over_var = 0;
}

function showQoSList(){
	var code = "";

	for(var i = 0; i < rule_desc.length; i++){
		code += '<a href="#"><div onmouseover="over_var=1;" onmouseout="over_var=0;" onclick="setClientIP('+i+');"><strong>'+rule_desc[i]+'</strong></div></a>';
	}
	code +='<!--[if lte IE 6.5]><iframe class="hackiframe2"></iframe><![endif]-->';	
	$("QoSList_Block").innerHTML = code;
}

function pullQoSList(obj){
	if(isMenuopen == 0){		
		obj.src = "/images/arrow-top.gif"
		$("QoSList_Block").style.display = 'block';		
		document.form.qos_service_name_x_0.focus();		
		isMenuopen = 1;
	}
	else
		hideClients_Block();
}

var over_var = 0;
var isMenuopen = 0;

function hideClients_Block(){
	$("pull_arrow").src = "/images/arrow-down.gif";
	$('QoSList_Block').style.display='none';
	isMenuopen = 0;
}
/*----------} Mouse event of fake LAN IP select menu-----------------*/

//Viz add 2011.11 for replace ">" to ":65535"   &   "<" to "1:"  {
function replace_symbol(){	
					var largre_src=new RegExp("^(>)([0-9]{1,5})$", "gi");
					if(largre_src.test(document.form.qos_port_x_0.value)){
						document.form.qos_port_x_0.value = document.form.qos_port_x_0.value.replace(/[>]/gi,"");	// ">" to ""
						document.form.qos_port_x_0.value = document.form.qos_port_x_0.value+":65535"; 						// add ":65535"
					}
					var smalre_src=new RegExp("^(<)([0-9]{1,5})$", "gi");
					if(smalre_src.test(document.form.qos_port_x_0.value)){
						document.form.qos_port_x_0.value = document.form.qos_port_x_0.value.replace(/[<]/gi,"");	// "<" to ""
						document.form.qos_port_x_0.value = "1:"+document.form.qos_port_x_0.value; 						// add "1:"						
					}		
}					
//} Viz add 2011.11 for replace ">" to ":65535"   &   "<" to "1:" 

</script>
</head>

<body onLoad="initial();" onunLoad="return unload_body();">
<div id="TopBanner"></div>
<div id="Loading" class="popup_bg"></div>

<iframe name="hidden_frame" id="hidden_frame" src="" width="0" height="0" frameborder="0"></iframe>

<form method="post" name="form" id="ruleForm" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="current_page" value="Advanced_QOSUserSpec_Content.asp">
<input type="hidden" name="next_page" value="">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_wait" value="5">
<input type="hidden" name="action_script" value="restart_qos">
<input type="hidden" name="first_time" value="">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="qos_rulelist" value=''>
<input type="hidden" name="qos_orates" value=''>
<input type="hidden" name="qos_irates" value=''>

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
		<td valign="top" >
			<table width="760px" border="0" cellpadding="4" cellspacing="0" class="FormTitle" id="FormTitle">			
			<tbody>
				<tr>
		  			<td bgcolor="#4D595D">
						<div style="margin-top:8px;" align="right">
	   					<select onchange="switchPage(this.options[this.selectedIndex].value)" class="input_option">
								<!--option><#switchpage#></option-->
								<option value="1"><#qos_automatic_mode#></option>
								<option value="2" selected>User define QoS rules</option>
							</select>	    
						</div>
		  			<div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
		  			<div class="formfontdesc"><#UserQoS_desc#></div>
		  
						<table width="100%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable">
							<thead>
							<tr>
								<td colspan="2"><#BM_status#></td>
							</tr>
							</thead>
							
							<tr>
								<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(20, 2);">Upload Bandwidth</a></th>
								<td><input type="text" maxlength="10" name="qos_obw" onblur="gen_options();" onKeyPress="return is_number_sp(event, this);" class="input_12_table" value="<% nvram_get("qos_obw"); %>" onBlur=""> kb/s</td>
							</tr>
							
							<tr>
								<th><a class="hintstyle" href="javascript:void(0);" onClick="openHint(20, 2);">Download Bandwidth</a></th>
								<td><input type="text" maxlength="10" name="qos_ibw" onblur="gen_options();" onKeyPress="return is_number_sp(event, this);" class="input_12_table" value="<% nvram_get("qos_ibw"); %>" onBlur=""> kb/s</td>
							</tr>
						</table>
						<table width="100%"  border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table" style="margin-top:8px">
							<thead>
							<tr>
								<td colspan="4" id="TriggerList" style="border-right:none;"><#BM_UserList_title#></td>
								<td colspan="3" id="TriggerList" style="border-left:none;">
									<div style="margin-top:0px;display:none" align="right">
										<select id='qos_default_sel' name='qos_default_sel' class="input_option" onchange="change_wizard(this);"></select>
									</div>
								</td>
							</tr>
							</thead>
			
							<tr>
								<th><#BM_UserList1#></th>
								<th><a href="javascript:void(0);" onClick="openHint(18,6);"><div class="table_text">Source IP or MAC</div></a></th> <!--a class="hintstyle" -->
								<th><a href="javascript:void(0);" onClick="openHint(18,4);"><div class="table_text"><#BM_UserList3#></div></a></th>
								<th><div class="table_text">Protocol</div></th>
								<th><a href="javascript:void(0);" onClick="openHint(18,5);"><div class="table_text"><div class="table_text">Transfered</div></a></th>
								<th><#BM_UserList4#></th>
								<th>Edit</th>
							</tr>							
							<tr>
								<td width="18%"><input type="text" class="input_12_table" style="float:left;" name="qos_service_name_x_0" onKeyPress="return is_string(this)" onblur="if(!over_var){hideClients_Block();}">
									<img id="pull_arrow" height="14px;" src="/images/arrow-down.gif" onclick="pullQoSList(this);" title="Select the device name of DHCP clients." onmouseover="over_var=1;" onmouseout="over_var=0;">
									<div id="QoSList_Block" class="QoSList_Block"></div>	
								</td>
								<td width="17%"><input type="text" maxlength="17" class="input_15_table" name="qos_ip_x_0" onblur="valid_IPorMAC(this)"></td><!--  onKeyPress="return is_iprange(this)" onKeyUp="change_iprange(this)"-->
								<td width="15%"><input type="text" class="input_12_table" name="qos_port_x_0" onKeyPress="return is_portrange(this)"></td>
								<td width="13%">
									<select name='qos_proto_x_0' class="input_option" style="width:87px;">
										<option value='tcp'>TCP</option>
										<option value='udp'>UDP</option>
										<option value='tcp/udp'>TCP/UDP</option>
										<option value='any'>ANY</option>
										<!--	marked By Viz 2011.12 for "iptables -p"
										option value='icmp'>ICMP</option>
										<option value='igmp'>IGMP</option -->
									</select>
								</td>
								<td width="16%">
									<input type="text" class="input_3_table" onKeyPress="return is_number_sp(event, this);" onblur="conv_to_transf();" name="qos_min_transfered_x_0">~
									<input type="text" class="input_3_table" onKeyPress="return is_number_sp(event, this);" onblur="conv_to_transf();" name="qos_max_transfered_x_0"> KB
									<input type="hidden" name="qos_transfered_x_0" value="">
								</td>
								<td width="13%">
									<select name='qos_prio_x_0' class="input_option" style="width:85px;">
										<option value='0'><#Highest#></option>
										<option value='1'><#High#></option>
										<option value='2' selected><#Medium#></option>
										<option value='3'><#Low#></option>
										<option value='4'><#Lowest#></option>
									</select>
								</td>
								
								<td width="8%">
									<input type="button" class="add_btn" onClick="addRow_Group(32);">
								</td>
							</tr>
							</table>
							
							<div id="qos_rulelist_Block"></div>				  	
					</td>
				</tr>

				<tr>
		  			<td bgcolor="#4D595D">
						<table width="100%"  border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable">
							<thead>
							<tr>
								<td><#highest_prio_packet#> &nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;&nbsp;
									<a href="#" id="packet_table_display_id" style="margin-left:490px;display:none;" onclick='bw_crtl_display("packet_table_display_id", "packet_table");'>-</a>
								</td>
							</tr>
							</thead>							

							<tr>
								<td>
									<div  id='packet_table' style='display:block;'>
										<table width="100%" border="0" cellpadding="4" cellspacing="0">
											<tr>
												<td style="font-size:12px; border-collapse: collapse;border:0;">		
													<input type="checkbox" name="qos_ack_checkbox" <% nvram_match("qos_ack", "on", "checked"); %>>ACK
													<input type="hidden" name="qos_ack">
												</td>
												<td style="font-size:12px; border-collapse: collapse;border:0;">
													<input type="checkbox" name="qos_syn_checkbox" <% nvram_match("qos_syn", "on", "checked"); %>>SYN
													<input type="hidden" name="qos_syn">
												</td>
												<td style="font-size:12px; border-collapse: collapse;border:0;">
													<input type="checkbox" name="qos_fin_checkbox" <% nvram_match("qos_fin", "on", "checked"); %>>FIN
													<input type="hidden" name="qos_fin">
												</td>
												<td style="font-size:12px; border-collapse: collapse;border:0;">
													<input type="checkbox" name="qos_rst_checkbox" <% nvram_match("qos_rst", "on", "checked"); %>>RST
													<input type="hidden" name="qos_rst">
												</td>
												<td style="font-size:12px; border-collapse: collapse;border:0;">
													<input type="checkbox" name="qos_icmp_checkbox" <% nvram_match("qos_icmp", "on", "checked"); %>>ICMP
													<input type="hidden" name="qos_icmp">
												</td>
											</tr>
										</table>
									</div>
								</td>
							</tr>
						</table>
					</td>
				</tr>

				<tr>
					<td bgcolor="#4D595D">		
						<table width="100%"  border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable">
							<thead>	
							<tr>
								<td colspan="2"><div><#set_rate_limit#><a href="#" id="bw_crtl_display_id" style="margin-left:490px;display:none;" onclick='bw_crtl_display("bw_crtl_display_id", "bandwidth_level");'>-</a></div></td>
							</tr>
							</thead>	

							<tr>
								<td>
									<div id='bandwidth_level' style='display:block'>
										<table  width="100%" border="0" cellpadding="4" cellspacing="0">
										<tr>
											<td width="58%" style="font-size:12px; border-collapse: collapse;border:0; padding-left:0;">			  
												<table width="100%" border="0" cellpadding="4" cellspacing="0" style="font-size:12px; border-collapse: collapse;border:0;">
												<thead>	
												<tr>
													<td colspan="4" >Upload Bandwidth:</td>
												</tr>
												<tr>
													<th style="width:22%;line-height:15px;color:#FFFFFF;"><#upload_prio#></th>
													<th style="width:25%;line-height:15px;color:#FFFFFF;"><a href="javascript:void(0);" onClick="openHint(20,3);"><div class="table_text"><#min_bound#></div></a></th>
													<th style="width:26%;line-height:15px;color:#FFFFFF;"><a href="javascript:void(0);" onClick="openHint(20,4);"><div class="table_text"><#max_bound#></div></a></th>
													<th style="width:27%;line-height:15px;color:#FFFFFF;"><#current_settings#></th>
												</tr>
												</thead>												
												<tr>
													<th style="width:22%;line-height:15px;"><#Highest#></th>
													<td> 
														<select name='upload_bw_min_0' class="input_option" id="upload_bw_min_0" onchange="gen_options();"></select>
													</td>	
													<td style="text-align:right;">
														<select name='upload_bw_max_0' class="input_option" id="upload_bw_max_0" onchange="gen_options();"></select>
													</td>
													<td>
														<div id="upload_bw_0_desc"></div>
													</td>
												</tr>
												<tr>
													<th style="width:22%;line-height:15px;"><#High#></th>
													<td>
														<select name='upload_bw_min_1' class="input_option" id="upload_bw_min_1" onchange="gen_options();"></select>
													</td>	
													<td style="text-align:right;">
														<select name='upload_bw_max_1' class="input_option" id="upload_bw_max_1" onchange="gen_options();"></select>
													</td>
													<td>
														<div id="upload_bw_1_desc"></div>
													</td>
												</tr>
												<tr>
													<th style="width:22%;line-height:15px;"><#Medium#></th>
													<td>
														<select name='upload_bw_min_2' class="input_option" id="upload_bw_min_2" onchange="gen_options();"></select>
													</td>	
													<td style="text-align:right;">
														<select name='upload_bw_max_2' class="input_option" id="upload_bw_max_2" onchange="gen_options();"></select>
													</td>
													<td>
														<div id="upload_bw_2_desc"></div>
													</td>
												</tr>
												<tr>
													<th style="width:22%;line-height:15px;"><#Low#></th>
													<td>
														<select name='upload_bw_min_3' class="input_option" id="upload_bw_min_3" onchange="gen_options();"></select>
													</td>	
													<td style="text-align:right;">
														<select name='upload_bw_max_3' class="input_option" id="upload_bw_max_3" onchange="gen_options();"></select>
													</td>
													<td>
														<div id="upload_bw_3_desc"></div>
													</td>
												</tr>
												<tr>
													<th style="width:22%;line-height:15px;"><#Lowest#></th>
													<td>
														<select name='upload_bw_min_4' class="input_option" id="upload_bw_min_4" onchange="gen_options();"></select>
													</td>	
													<td style="text-align:right;">
														<select name='upload_bw_max_4' class="input_option" id="upload_bw_max_4" onchange="gen_options();"></select>
													</td>
													<td>
														<div id="upload_bw_4_desc"></div>
													</td>
												</tr>
												</table>
											</td>

											<td width="42%" style="font-size:12px; border-collapse: collapse;border:0;">
												<table width="100%" border="0" cellpadding="4" cellspacing="0" style="font-size:12px; border-collapse: collapse;border:0;">
												<thead>
												<tr>
													<td colspan="3">Download Bandwidth:</td>
												</tr>
												<tr>
													<th style="width:31%;line-height:15px;color:#FFFFFF;"><#download_prio#></th>
													<th style="width:37%;line-height:15px;color:#FFFFFF;"><a href="javascript:void(0);" onClick="openHint(20,5);"><div class="table_text"><#max_bound#></div></a></th>
													<th style="width:32%;line-height:15px;color:#FFFFFF;"><#current_settings#></th>
												</tr>
												</thead>
												<tr>
													<th style="width:31%;line-height:15px;"><#Highest#></th>
													<td style="text-align:right;"> 
														<select name='download_bw_min_0' class="input_option" id="download_bw_min_0" onchange="gen_options();"></select>
														<!--select name='download_bw_max_0' class="input_option" id="download_bw_max_0"></select-->
													</td>
													<td>
														<div id="download_bw_0_desc"></div>
													</td>
												</tr>
												<tr>
													<th style="width:31%;line-height:15px;"><#High#></th>
													<td style="text-align:right;">
														<select name='download_bw_min_1' class="input_option" id="download_bw_min_1" onchange="gen_options();"></select>
														<!--select name='download_bw_max_1' class="input_option" id="download_bw_max_1"></select-->
													</td>
													<td>
														<div id="download_bw_1_desc"></div>
													</td>
												</tr>
												<tr>
													<th style="width:31%;line-height:15px;"><#Medium#></th>
													<td style="text-align:right;">			  
														<select name='download_bw_min_2' class="input_option" id="download_bw_min_2" onchange="gen_options();"></select>
														<!--select name='download_bw_max_2' class="input_option" id="download_bw_max_2"></select-->
													</td>
													<td>
														<div id="download_bw_2_desc"></div>
													</td>
												</tr>
												<tr>
													<th style="width:31%;line-height:15px;"><#Low#></th>
													<td style="text-align:right;">
														<select name='download_bw_min_3' class="input_option" id="download_bw_min_3" onchange="gen_options();"></select>
														<!--select name='download_bw_max_3' class="input_option" id="download_bw_max_3"></select-->
													</td>
													<td>
														<div id="download_bw_3_desc"></div>
													</td>
												</tr>
												<tr>
													<th style="width:31%;line-height:15px;"><#Lowest#></th>
													<td style="text-align:right;">
														<select name='download_bw_min_4' class="input_option" id="download_bw_min_4" onchange="gen_options();"></select>
														<!--select name='download_bw_max_4' class="input_option" id="download_bw_max_4"></select-->
													</td>
													<td>
														<div id="download_bw_4_desc"></div>
													</td>
												</tr>
												</table>
											</td>
										</tr>
										</table>
									</div>
								</td>
							</tr>		  
							</table>
					<div class="apply_gen">
						<input name="button" type="button" class="button_gen" onClick="applyRule()" value="<#CTL_apply#>"/>
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

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
<title><#Web_Title#> - <#menu3#></title>
<link rel="stylesheet" type="text/css" href="index_style.css"> 
<link rel="stylesheet" type="text/css" href="form_style.css">
<script type="text/javascript" src="/disk_functions.js"></script>
<style type="text/css">
/* folder tree */
.mask_bg{
	position:absolute;	
	margin:auto;
	top:0;
	left:0;
	width:100%;
	height:100%;
	z-index:100;
	background:url(/images/popup_bg2.gif);
	background-repeat: repeat;
	filter:progid:DXImageTransform.Microsoft.Alpha(opacity=60);
	-moz-opacity: 0.6;
	display:none;
	overflow:hidden;
}
.mask_floder_bg{
	position:absolute;	
	margin:auto;
	top:0;
	left:0;
	width:100%;
	height:100%;
	z-index:300;
	background:url(/images/popup_bg2.gif);
	background-repeat: repeat;
	filter:progid:DXImageTransform.Microsoft.Alpha(opacity=60);
	-moz-opacity: 0.6;
	display:none;
	overflow:hidden;
}
.panel{
	width:450px;
	position:absolute;
	margin-top:-8%;
	margin-left:35%;
	z-index:200;
	display:none;
}
.floder_panel{
	background-color:#999;	
	border:2px outset #CCC;
	font-size:15px;
	font-family:Verdana, Geneva, sans-serif;
	color:#333333;
	width:450px;
	position:absolute;
	margin-top:-8%;
	margin-left:35%;
	z-index:400;
	display:none;
}
.folderClicked{
	color:#569AC7;
	font-size:14px;
	cursor:text;
}
.lastfolderClicked{
	color:#FFFFFF;
	cursor:pointer;
}
.panel_new{
	font-family:Courier ;
	width:500px;
	position:absolute;
	margin-left:35%;
	z-index:2000;
	display:none;
	background-image:url(images/Tree/bg_01.png);
	background-repeat:no-repeat;
}
#status_gif_Img_L{
	background-image:url(images/cloudsync/left_right_trans.gif);
	background-position: 10px -0px; width: 59px; height: 38px;
}
#status_gif_Img_LR{
	background-image:url(images/cloudsync/left_right_trans.gif);
	background-position: 10px -47px; width: 59px; height: 38px;
}
#status_gif_Img_R{
	background-image:url(images/cloudsync/left_right_trans.gif);
	background-position: 10px -97px; width: 59px; height: 38px;
}

#status_png_Img_error{
	background-image:url(images/cloudsync/left_right_done.png);
	background-position: -0px -0px; width: 59px; height: 38px;
}
#status_png_Img_L_ok{
	background-image:url(images/cloudsync/left_right_done.png);
	background-position: -0px -47px; width: 59px; height: 38px;
}
#status_png_Img_R_ok{
	background-image:url(images/cloudsync/left_right_done.png);
	background-position: -0px -95px; width: 59px; height: 38px;
}
#status_png_Img_LR_ok{
	background-image:url(images/cloudsync/left_right_done.png);
	background-position: -0px -142px; width: 59px; height: 38px;
}
</style>
<script type="text/javascript" src="/state.js"></script>
<script type="text/javascript" src="/popup.js"></script>
<script type="text/javascript" src="/help.js"></script>
<script type="text/javascript" src="/detect.js"></script>
<script type="text/javascript" src="/jquery.js"></script>
<script type="text/javascript" src="/switcherplugin/jquery.iphone-switch.js"></script>
<script>
var $j = jQuery.noConflict();
<% login_state_hook(); %>
var wireless = [<% wl_auth_list(); %>];	// [[MAC, associated, authorized], ...]
wan_route_x = '<% nvram_get("wan_route_x"); %>';
wan_nat_x = '<% nvram_get("wan_nat_x"); %>';
wan_proto = '<% nvram_get("wan_proto"); %>';

var cloud_sync;
var cloud_status = "";
var cloud_obj = "";
var cloud_msg = "";
var curRule = -1;
var enable_cloudsync = '<% nvram_get("enable_cloudsync"); %>';

<% cloud_status(); %>

/* type>user>password>url>dir>rule>enable */
var cloud_synclist_array = cloud_sync.replace(/>/g, "&#62").replace(/</g, "&#60"); 
var isEdit = 0;
var isonEdit = 0;
var maxrulenum = 1;
var rulenum = 1;

function initial(){
	show_menu();
	showAddTable();
	showcloud_synclist();
	_initial_dir();
}

function _initial_dir(){
	var __layer_order = "0_0";
	var url = "/getfoldertree.asp";
	var type = "General";

	url += "?motion=gettree&layer_order=" + __layer_order + "&t=" + Math.random();
	$j.get(url,function(data){_initial_dir_status(data);});
}

function _initial_dir_status(data){
	if(data != ""){
		eval("var default_dir=" + data);
		document.form.cloud_dir.value = "/mnt/" + default_dir.substr(0, default_dir.indexOf("#")) + "/MySyncFolder";
	}
	else{
		$("noUSB").style.display = "";
	}
}

function addRow(obj, head){
	if(head == 1)
		cloud_synclist_array += "&#62";
	else
		cloud_synclist_array += "&#60";
			
	cloud_synclist_array += obj.value;
	obj.value = "";
}

function addRow_Group(upper){ 
	var rule_num = $('cloud_synclist_table').rows.length;
	var item_num = $('cloud_synclist_table').rows[0].cells.length;
	
	if(rule_num >= upper){
		alert("<#JS_itemlimit1#> " + upper + " <#JS_itemlimit2#>");
		return false;	
	}			
				
	Do_addRow_Group();		
}

function Do_addRow_Group(){		
	addRow(document.form.cloud_sync_type ,1);
	addRow(document.form.cloud_sync_username, 0);
	addRow(document.form.cloud_sync_password, 0);
	addRow(document.form.cloud_sync_dir, 0);
	addRow(document.form.cloud_sync_rule, 0);
	showcloud_synclist();
}

function edit_Row(r){ 	
	var i=r.parentNode.parentNode.rowIndex;
  	
	document.form.cloud_sync_type.value = $('cloud_synclist_table').rows[i].cells[0].innerHTML;
	document.form.cloud_sync_username.value = $('cloud_synclist_table').rows[i].cells[1].innerHTML; 
	document.form.cloud_sync_password.value = $('cloud_synclist_table').rows[i].cells[2].innerHTML; 
	document.form.cloud_sync_dir.value = $('cloud_synclist_table').rows[i].cells[3].innerHTML;
	document.form.cloud_sync_rule.value = $('cloud_synclist_table').rows[i].cells[4].innerHTML;

  del_Row(r);	
}

function del_Row(r){
	if(isonEdit == 1)
		return false;

	var cloud_synclist_row = cloud_synclist_array.split('&#60'); // resample
  var i=r.parentNode.parentNode.rowIndex+1;

  var cloud_synclist_value = "";
	for(k=1; k<cloud_synclist_row.length; k++){
		if(k == i)
			continue;
		else
			cloud_synclist_value += "&#60";

		var cloud_synclist_col = cloud_synclist_row[k].split('&#62');
		for(j=0; j<cloud_synclist_col.length-1; j++){
			cloud_synclist_value += cloud_synclist_col[j];
			if(j != cloud_synclist_col.length-1)
				cloud_synclist_value += "&#62";
		}
	}

	isonEdit = 1;
	cloud_synclist_array = cloud_synclist_value;
	showcloud_synclist();
	document.form.cloud_sync.value = cloud_synclist_array.replace(/&#62/g, ">").replace(/&#60/g, "<");
	$("update_scan").style.display = '';
	FormActions("start_apply.htm", "apply", "restart_cloudsync", "2");
	showLoading();
	document.form.submit();
}

function showcloud_synclist(){
	if(cloud_synclist_array != ""){
		$("creatBtn").style.display = "none";
	}

	rulenum = 0;
	var cloud_synclist_row = cloud_synclist_array.split('&#60');
	var code = "";

	code +='<table width="99%" cellspacing="0" cellpadding="4" align="center" class="list_table" id="cloud_synclist_table">';
	if(enable_cloudsync == '0')
		code +='<tr height="55px"><td style="color:#FFCC00;" colspan="6"><#nosmart_sync#></td>';
	else if(cloud_synclist_array == "")
		code +='<tr height="55px"><td style="color:#FFCC00;" colspan="6"><#IPConnection_VSList_Norule#></td>';
	else{
		for(var i = 0; i < cloud_synclist_row.length; i++){
			rulenum++;
			code +='<tr id="row'+i+'" height="55px">';
			var cloud_synclist_col = cloud_synclist_row[i].split('&#62');
			var wid = [10, 25, 0, 0, 10, 30, 15];
			for(var j = 0; j < cloud_synclist_col.length; j++){
				if(j == 2 || j == 3){
					continue;
				}
				else{
					if(j == 0)
						code +='<td width="'+wid[j]+'%"><span style="display:none">'+ cloud_synclist_col[j] +'</span><img width="30px" src="/images/cloudsync/ASUS-WebStorage.png"></td>';
					else if(j == 1)
						code +='<td width="'+wid[j]+'%"><span style="display:none">'+ cloud_synclist_col[j] +'</span><span style="font-size:16px;font-family: Calibri;font-weight: bolder;">'+ cloud_synclist_col[j] +'</span></td>';
					else if(j == 4){
						curRule = cloud_synclist_col[j];
						if(cloud_synclist_col[j] == 2)
							code +='<td width="'+wid[j]+'%"><span style="display:none">'+ cloud_synclist_col[j] +'</span><div id="status_image"><div id="status_gif_Img_L"></div></div></td>';//code +='<td width="'+wid[j]+'%"><span style="display:none">'+ cloud_synclist_col[j] +'</span><img id="statusImg" width="45px" src="/images/cloudsync/left.gif"></td>';
						else if(cloud_synclist_col[j] == 1)
							code +='<td width="'+wid[j]+'%"><span style="display:none">'+ cloud_synclist_col[j] +'</span><div id="status_image"><div id="status_gif_Img_R"></div></div></td>';
						else
							code +='<td width="'+wid[j]+'%"><span style="display:none">'+ cloud_synclist_col[j] +'</span><div id="status_image"><div id="status_gif_Img_LR"></div></div></td>';
					}
					else if(j == 6){
						code +='<td width="'+wid[j]+'%" id="cloudStatus"></td>';
					}
					else{
						code +='<td width="'+wid[j]+'%"><span style="display:none;">'+ cloud_synclist_col[j] +'</span><span style="word-break:break-all;">'+ cloud_synclist_col[j].substr(4, cloud_synclist_col[j].length) +'</span></td>';
					}
				}
			}
			code +='<td width="10%"><input class="remove_btn" onclick="del_Row(this);" value=""/></td>';
		}
		updateCloudStatus();
	}
  code +='</table>';
	$("cloud_synclist_Block").innerHTML = code;
}

function updateCloudStatus(){
    $j.ajax({
    	url: '/update_cloudstatus.asp',
    	dataType: 'script', 

    	error: function(xhr){
      		updateCloudStatus();
    	},
    	success: function(response){
					if(cloud_status.toUpperCase() == "DOWNUP"){
						cloud_status = "SYNC";
						$("status_image").firstChild.id="status_gif_Img_LR";
						//$("statusImg").src = "/images/cloudsync/left_right.gif";
					}
					else if(cloud_status.toUpperCase() == "ERROR"){
						$("status_image").firstChild.id="status_png_Img_error";
						//$("statusImg").src = "/images/cloudsync/left_right_Error.png";
					}
					else if(cloud_status.toUpperCase() == "UPLOAD"){
						$("status_image").firstChild.id="status_gif_Img_L";
						//$("statusImg").src = "/images/cloudsync/left.gif";
					}
					else if(cloud_status.toUpperCase() == "DOWNLOAD"){
						$("status_image").firstChild.id="status_gif_Img_R";
						//$("statusImg").src = "/images/cloudsync/right.gif";
					}
					else if(cloud_status.toUpperCase() == "SYNC"){
						cloud_status = "Finish";
						if(curRule == 2){
							$("status_image").firstChild.id="status_png_Img_L_ok";
							//$("statusImg").src = "/images/cloudsync/left_ok.png";
						}else if(curRule == 1){
							$("status_image").firstChild.id="status_png_Img_R_ok";
							//$("statusImg").src = "/images/cloudsync/right_ok.png";
						}else{
							$("status_image").firstChild.id="status_png_Img_LR_ok";
							//$("statusImg").src = "/images/cloudsync/left_right_ok.png";
						}	
					}

					// handle msg
					var _cloud_msg = "";
					if(cloud_obj != ""){
						_cloud_msg +=  "<b>";
						_cloud_msg += cloud_status;
						_cloud_msg += ": </b><br />";
						_cloud_msg += "<span style=\\'word-break:break-all;\\'>" + decodeURIComponent(cloud_obj) + "</span>";
					}
					else if(cloud_msg){
						_cloud_msg += cloud_msg;
					}
					else{
						_cloud_msg += "No log.";
					}

					// handle status
					var _cloud_status;
					if(cloud_status != "")
						_cloud_status = cloud_status;
					else
						_cloud_status = "";

					if($("cloudStatus"))
						$("cloudStatus").innerHTML = '<div style="text-decoration:underline; cursor:pointer" onmouseout="return nd();" onclick="return overlib(\''+ _cloud_msg +'\');">'+ _cloud_status +'</div>';

			 		setTimeout("updateCloudStatus();", 2000);
      }
   });
}

function validform(){
	if(document.form.cloud_username.value == ''){
		alert("<#File_Pop_content_alert_desc1#>");
		return false;
	}

	if(document.form.cloud_password.value == ''){
		alert("<#File_Pop_content_alert_desc6#>");
		return false;
	}

	if(document.form.cloud_dir.value.split("/").length < 4 || document.form.cloud_dir.value == ''){
		alert("<#ALERT_OF_ERROR_Input10#>");
		return false;
	}

	return true;
}

function applyRule(){
	if(validform()){
		isonEdit = 1;
		cloud_synclist_array += '0&#62'+document.form.cloud_username.value+'&#62'+document.form.cloud_password.value+'&#62none&#62'+document.form.cloud_rule.value+'&#62'+"/tmp"+document.form.cloud_dir.value+'&#621';
		showcloud_synclist();
	
		document.form.cloud_sync.value = cloud_synclist_array.replace(/&#62/g, ">").replace(/&#60/g, "<");
		document.form.cloud_username.value = '';
		document.form.cloud_password.value = '';
		document.form.cloud_rule.value = '';
		document.form.cloud_dir.value = '';
		document.form.enable_cloudsync.value = 1;

		isEdit = 0;
		showAddTable();
		$("update_scan").style.display = '';
		FormActions("start_apply.htm", "apply", "restart_cloudsync", "2");
		showLoading();
		document.form.submit();
	}
}

function showAddTable(){
	if(isEdit == 1){ // edit
		$j("#cloudAddTable").fadeIn();
		$("creatBtn").style.display = "none";
		$("applyBtn").style.display = "";
	}
	else{ // list
		$("cloudAddTable").style.display = "none";
		$("creatBtn").style.display = "";
		$("applyBtn").style.display = "none";
	}
}

// get folder tree
var dm_dir = new Array(); 
var Download_path = '/mnt/';
var WH_INT=0,Floder_WH_INT=0,General_WH_INT=0;
var BASE_PATH;
var folderlist = new Array();

function showPanel(){
	WH_INT = setInterval("getWH();",1000);
 	$j("#DM_mask").fadeIn(1000);
  $j("#panel_add").show(1000);
	create_tree();
}

function getWH(){
	var winWidth;
	var winHeight;
	winWidth = document.documentElement.scrollWidth;
	if(document.documentElement.clientHeight > document.documentElement.scrollHeight)
		winHeight = document.documentElement.clientHeight;
	else
		winHeight = document.documentElement.scrollHeight;
	$("DM_mask").style.width = winWidth+"px";
	$("DM_mask").style.height = winHeight+"px";
}

function getFloderWH(){
	var winWidth;
	var winHeight;
	winWidth = document.documentElement.scrollWidth;

	if(document.documentElement.clientHeight > document.documentElement.scrollHeight)
		winHeight = document.documentElement.clientHeight;
	else
		winHeight = document.documentElement.scrollHeight;

	$("DM_mask_floder").style.width = winWidth+"px";
	$("DM_mask_floder").style.height = winHeight+"px";
}

function show_AddFloder(){
	Floder_WH_INT = setInterval("getFloderWH();",1000);
	$j("#DM_mask_floder").fadeIn(1000);
	$j("#panel_addFloder").show(1000);
}

function hidePanel(){
	($j("#tree").children()).remove();
	clearInterval(WH_INT);
	$j("#DM_mask").fadeOut('fast');
	$j("#panel_add").hide('fast');
}

function create_tree(){
	var rootNode = new Ext.tree.TreeNode({ text:'/mnt', id:'0'});
	var rootNodechild = new Ext.tree.TreeNode({ text:'', id:'0t'});
	rootNode.appendChild(rootNodechild);
	var tree = new Ext.tree.TreePanel({
			tbar:[{text:"<#CTL_ok#>",handler:function(){$j("#PATH").attr("value",Download_path);hidePanel();}},
				'->',{text:'X',handler:function(){hidePanel();}}
			],
			title:"Please select the desire folder",
				applyTo:'tree',
				root:rootNode,
				height:400,
				autoScroll:true
	});
	tree.on('expandnode',function(node){
		var allParentNodes = getAllParentNodes(node);
		var path='';

		for(var j=0; j<allParentNodes.length; j++){
			path = allParentNodes[j].text + '/' +path;
		}
		initial_dir(path,node);
	});
	tree.on('collapsenode',function(node){
		while(node.firstChild){
			node.removeChild(node.firstChild);
		}
		var childNode = new Ext.tree.TreeNode({ text:'', id:'0t'});
		node.appendChild(childNode);
	});
	tree.on('click',function(node){
		var allParentNodes = getAllParentNodes(node);
		var path='';
		for(var j=0; j<allParentNodes.length; j++){
			if(j == allParentNodes.length-2)
				continue;
			else
				path = allParentNodes[j].text + '/' +path;
		}

		Download_path = path;
		path = BASE_PATH + '/' + path;
		var url = "getfoldertree.asp";
		var type = "General";

		url += "?motion=gettree&layer_order="+ _layer_order +"&t=" +Math.random();
		$j.get(url,function(data){initial_folderlist(data);});
	});
}

function initial_folderlist(data){
	eval("folderlist=["+data+"]");
}

function getAllParentNodes(node) {
	var parentNodes = [];
	var _nodeID = node.id;
	_layer_order = "0";

	for(i=1; i<_nodeID.length; i++){
		_layer_order += "_" + node.id.charAt(i); // generating _layer_order for initial_dir() via charAt() method
	}
	parentNodes.push(node);
	while (node.parentNode) {
		parentNodes = parentNodes.concat(node.parentNode);
		node = node.parentNode;
	}
	return parentNodes;
};

function initial_dir(path,node){
	var url = "getfoldertree.asp";
	var type = "General";
	url += "?motion=gettree&layer_order="+ _layer_order +"&t=" +Math.random();
	$j.get(url,function(data){initial_dir_status(data,node);});
}

function initial_dir_status(data,node){
	dm_dir.length = 0;
	if(data == "/" || (data != null && data != "")){
		eval("dm_dir=[" + data +"]");	
		while(node.lastChild &&(node.lastChild !=node.firstChild)) {
    			node.removeChild(node.lastChild);
		}
		for(var i=0; i<dm_dir.length; i++){
			var childNodeId = node.id +i;
			var childnode = new Ext.tree.TreeNode({id:childNodeId,text:dm_dir[i].split("#")[0]});
			node.appendChild(childnode);
			var childnodeT = new Ext.tree.TreeNode({id:childNodeId+'t',text:''});
			childnode.appendChild(childnodeT);
		}
		node.removeChild(node.firstChild);
	}
	else{
		while(node.firstChild){
			node.removeChild(node.firstChild);
		}
	}
}
function get_disk_tree(){
  $j("#test_panel").fadeIn(300);
		get_layer_items("0");
}
function get_layer_items(layer_order){
	$j.ajax({
    		url: '/gettree.asp?layer_order='+layer_order,
    		dataType: 'script',
    		error: function(xhr){
    			;
    		},
    		success: function(){
				get_tree_items(treeitems);					
  			}
		});
}
function get_tree_items(treeitems){
	this.isLoading = 1;
	this.Items = treeitems;
		if(this.Items && this.Items.length > 0){
			BuildTree();
		}	
}
function BuildTree(){
	var ItemText, ItemSub, ItemIcon;
	var vertline, isSubTree;
	var layer;
	var short_ItemText = "";
	var shown_ItemText = "";
	var ItemBarCode ="";
		
	var TempObject = "";
	for(var i = 0; i < this.Items.length; ++i){
	
		this.Items[i] = this.Items[i].split("#");
		var Item_size = 0;
		Item_size = this.Items[i].length;
		if(Item_size > 3){
			var temp_array = new Array(3);
			
			temp_array[2] = this.Items[i][Item_size-1];
			temp_array[1] = this.Items[i][Item_size-2];
			
			temp_array[0] = "";
			for(var j = 0; j < Item_size-2; ++j){
				if(j != 0)
					temp_array[0] += "#";
				temp_array[0] += this.Items[i][j];
			}
			this.Items[i] = temp_array;
		}
	
		ItemText = (this.Items[i][0]).replace(/^[\s]+/gi,"").replace(/[\s]+$/gi,"");
		ItemBarCode = this.FromObject+"_"+(this.Items[i][1]).replace(/^[\s]+/gi,"").replace(/[\s]+$/gi,"");
		ItemSub = parseInt((this.Items[i][2]).replace(/^[\s]+/gi,"").replace(/[\s]+$/gi,""));
		layer = get_layer(ItemBarCode.substring(1));
		if(layer == 3){
			if(ItemText.length > 21)
		 		short_ItemText = ItemText.substring(0,18)+"...";
		 	else
		 		short_ItemText = ItemText;
		}
		else
			short_ItemText = ItemText;
		
		shown_ItemText = showhtmlspace(short_ItemText);
		
		if(layer == 1)
			ItemIcon = 'disk';
		else if(layer == 2)
			ItemIcon = 'part';
		else
			ItemIcon = 'folders';
		
		SubClick = ' onclick="GetFolderItem(this, ';
		if(ItemSub <= 0){
			SubClick += '0);"';
			isSubTree = 'n';
		}
		else{
			SubClick += '1);"';
			isSubTree = 's';
		}
		
		if(i == this.Items.length-1){
			vertline = '';
			isSubTree += '1';
		}
		else{
			vertline = ' background="/images/Tree/vert_line.gif"';
			isSubTree += '0';
		}
		
	
		TempObject += 
'<table class="tree_table" id="bug_test">\n'+
'<tr>\n'+
	// the line in the front.
	'<td class="vert_line">\n'+ 
		'<img id="a'+ItemBarCode+'" onclick=\'$("d'+ItemBarCode+'").onclick();\' class="FdRead" src="/images/Tree/vert_line_'+isSubTree+'0.gif">\n'+
	'</td>\n';
	
		if(layer == 3){
			TempObject += 		/*a: connect_line b: harddisc+name  c:harddisc  d:name e: next layer forder*/
	'<td>\n'+		
			'<img id="c'+ItemBarCode+'" onclick=\'$("d'+ItemBarCode+'").onclick();\' src="/images/New_ui/advancesetting/'+ItemIcon+'.png">\n'+
	'</td>\n'+	
	'<td>\n'+
			'<span id="d'+ItemBarCode+'"'+SubClick+' title="'+ItemText+'">'+shown_ItemText+'</span>\n'+		
	'</td>\n';

		}
		else if(layer == 2){
			TempObject += 
	'<td>\n';
			
			TempObject += 
'<table class="tree_table">\n'+
'<tr>\n';
			
			TempObject += 
	'<td class="vert_line">\n'+
			'<img id="c'+ItemBarCode+'" onclick=\'$("d'+ItemBarCode+'").onclick();\' src="/images/New_ui/advancesetting/'+ItemIcon+'.png">\n'+
	'</td>\n'+
	'<td class="FdText">\n'+
			'<span id="d'+ItemBarCode+'"'+SubClick+' title="'+ItemText+'">'+shown_ItemText+'</span>\n'+
	'</td>\n';
			
	
				TempObject += 
	'<td></td>';
			
			TempObject += 
'</tr>\n'+
'</table>\n';
			
			TempObject += 
	'</td>\n'+
'</tr>\n';
			
			TempObject += 
'<tr><td></td>\n';
			
			TempObject += 
	'<td colspan=2><div id="e'+ItemBarCode+'" ></div></td>\n';
		}
		else{
			TempObject += 		/*a: connect_line b: harddisc+name  c:harddisc  d:name e: next layer forder*/
	'<td>\n'+
	//	'<div id="b'+ItemBarCode+'" class="FdText">\n'+		/* style="float:left; width:117px; overflow:hidden;"*/
	'<table><tr><td>\n'+
			'<img id="c'+ItemBarCode+'" onclick=\'$("d'+ItemBarCode+'").onclick();\' src="/images/New_ui/advancesetting/'+ItemIcon+'.png">\n'+
	'</td><td>\n'+		
			'<span id="d'+ItemBarCode+'"'+SubClick+' title="'+ItemText+'">'+shown_ItemText+'</span>\n'+
	'</td></tr></table>\n'+		
		//'</div>\n'+
	'</td>\n'+
'</tr>\n';
			
			TempObject += 
'<tr><td></td>\n';
			
			TempObject += 
	'<td><div id="e'+ItemBarCode+'" ></div></td>\n';
		}
		
		TempObject += 
'</tr>\n';
	}
	
	TempObject += 
'</table>\n';
	
	$("e"+this.FromObject).innerHTML = TempObject;
	
}
var FromObject = "0";
var lastClickedObj = 0;
var _layer_order = "";
function get_layer(barcode){
	var tmp, layer;
	layer = 0;
	while(barcode.indexOf('_') != -1){
		barcode = barcode.substring(barcode.indexOf('_'), barcode.length);
		++layer;
		barcode = barcode.substring(1);		
	}
	return layer;
}
function build_array(obj,layer){
var path_temp ="/mnt";
var layer2_path ="";
var layer3_path ="";
	if(obj.id.length>6){
		if(layer ==3){
			layer3_path = "/" + $(obj.id).innerHTML;
			while(layer3_path.indexOf("&nbsp;") != -1)
				layer3_path = layer3_path.replace("&nbsp;"," ");
				if(obj.id.length >8)
					layer2_path = "/" + $(obj.id.substring(0,obj.id.length-3)).innerHTML;
				else
					layer2_path = "/" + $(obj.id.substring(0,obj.id.length-2)).innerHTML;
			
			while(layer2_path.indexOf("&nbsp;") != -1)
				layer2_path = layer2_path.replace("&nbsp;"," ");
		}
	}
	if(obj.id.length>4 && obj.id.length<=6){

		if(layer ==2){
			layer2_path = "/" + $(obj.id).innerHTML;
			while(layer2_path.indexOf("&nbsp;") != -1)
				layer2_path = layer2_path.replace("&nbsp;"," ");
		}
	}
	path_temp = path_temp + layer2_path +layer3_path;
	return path_temp;
}
function GetFolderItem(selectedObj, haveSubTree){
	//var path_directory;
	
	var barcode, layer = 0;
	showClickedObj(selectedObj);
	barcode = selectedObj.id.substring(1);	
	layer = get_layer(barcode);

	if(layer == 0)
		alert("Machine: Wrong");
	else if(layer == 1){
		// chose Disk
		setSelectedDiskOrder(selectedObj.id);
		path_directory = build_array(selectedObj,layer);	
	}
	else if(layer == 2){
		// chose Partition
		setSelectedPoolOrder(selectedObj.id);
		path_directory = build_array(selectedObj,layer);		
	}
	else if(layer == 3){
		// chose Shared-Folder
		setSelectedFolderOrder(selectedObj.id);
		path_directory = build_array(selectedObj,layer);
	}

	if(haveSubTree)
		GetTree(barcode, 1);
}
function showClickedObj(clickedObj){
	if(this.lastClickedObj != 0)
		this.lastClickedObj.className = "lastfolderClicked";  //this className set in AiDisk_style.css
	
	clickedObj.className = "folderClicked";

	this.lastClickedObj = clickedObj;
}
function GetTree(layer_order, v){
	if(layer_order == "0"){
		this.FromObject = layer_order;
		$('d'+layer_order).innerHTML = '<span class="FdWait">. . . . . . . . . .</span>';
		setTimeout('get_layer_items("'+layer_order+'", "gettree")', 1);		
		return;
	}
	
	if($('a'+layer_order).className == "FdRead"){
		$('a'+layer_order).className = "FdOpen";
		$('a'+layer_order).src = "/images/Tree/vert_line_s"+v+"1.gif";
		
		this.FromObject = layer_order;
		
		$('e'+layer_order).innerHTML = '<img src="/images/Tree/folder_wait.gif">';
		setTimeout('get_layer_items("'+layer_order+'", "gettree")', 1);
	}
	else if($('a'+layer_order).className == "FdOpen"){
		$('a'+layer_order).className = "FdClose";
		$('a'+layer_order).src = "/images/Tree/vert_line_s"+v+"0.gif";
		
		$('e'+layer_order).style.position = "absolute";
		$('e'+layer_order).style.visibility = "hidden";
	}
	else if($('a'+layer_order).className == "FdClose"){
		$('a'+layer_order).className = "FdOpen";
		$('a'+layer_order).src = "/images/Tree/vert_line_s"+v+"1.gif";
		
		$('e'+layer_order).style.position = "";
		$('e'+layer_order).style.visibility = "";
	}
	else
		alert("Error when show the folder-tree!");
}
function cancel_temp(){
this.FromObject ="";
$j("#test_panel").fadeOut(300);
}
function confirm_temp(){
	$('PATH').value = path_directory ;
	this.FromObject ="";
	$j("#test_panel").fadeOut(300);
}


var isNotIE = (navigator.userAgent.search("MSIE") == -1); 
function switchType(_method){
	if(isNotIE){
		document.form.cloud_password.type = _method ? "text" : "password";		
	}
}

function switchType_IE(obj){
		if(isNotIE) return;		
		
		var tmp = "";
		tmp = obj.value;
		if(obj.id.indexOf('text') < 0){		//password
							obj.style.display = "none";
							document.getElementById('cloud_password_text').style.display = "";
							document.getElementById('cloud_password_text').value = tmp;						
							document.getElementById('cloud_password_text').focus();
		}else{														//text					
							obj.style.display = "none";
							document.getElementById('cloud_password').style.display = "";
							document.getElementById('cloud_password').value = tmp;
		}
}
</script>
</head>

<body onload="initial();" onunload="return unload_body();">
<div id="TopBanner"></div>

<!-- floder tree-->
<div id="DM_mask" class="mask_bg"></div>
<div id="panel_add" class="panel">
	<div id="tree"></div>
</div>
<div id="test_panel" class="panel_new" >
		<div class="machineName" style="font-family:Microsoft JhengHei;font-size:12pt;font-weight:bolder; margin-top:25px;margin-left:30px;"><#Web_Title2#></div>
		<div id="e0" class="FdTemp" style="font-size:10pt; margin-top:8px;margin-left:30px;margin-bottom:1px;height:345px;overflow:auto;width:455px;"></div>
	<div style="background-image:url(images/Tree/bg_02.png);background-repeat:no-repeat;height:90px;">		
		<input class="button_gen" type="button" style="margin-left:27%;margin-top:18px;" onclick="cancel_temp();" value="<#CTL_Cancel#>">
		<input class="button_gen" type="button"  onclick="confirm_temp();" value="<#CTL_ok#>">
	</div>
</div>
<div id="DM_mask_floder" class="mask_floder_bg"></div>
<div id="panel_addFloder" class="floder_panel">
	<span style="margin-left:95px;"><b id="multiSetting_0"></b></span><br /><br />
	<span style="margin-left:8px;margin-right:8px;"><b id="multiSetting_1"></b></span>
	<input type="text" id="newFloder" class="input_15_table" value="" /><br /><br />
	<input type="button" name="AddFloder" id="multiSetting_2" value="" style="margin-left:100px;" onclick="AddFloderName();">
	&nbsp;&nbsp;
	<input type="button" name="Cancel_Floder_add" id="multiSetting_3" value="" onClick="hide_AddFloder();">
</div>
<!-- floder tree-->

<div id="Loading" class="popup_bg"></div>
<iframe name="hidden_frame" id="hidden_frame" width="0" height="0" frameborder="0" scrolling="no"></iframe>
<form method="post" name="form" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="current_page" value="cloud_sync.asp">
<input type="hidden" name="next_page" value="cloud_sync.asp">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="restart_cloudsync">
<input type="hidden" name="action_wait" value="1">
<input type="hidden" name="cloud_sync" value="">
<input type="hidden" name="enable_cloudsync" value="<% nvram_get("enable_cloudsync"); %>">

<table border="0" align="center" cellpadding="0" cellspacing="0" class="content">
	<tr>
		<td valign="top" width="17">&nbsp;</td>
		<!--=====Beginning of Main Menu=====-->
		<td valign="top" width="202">
			<div id="mainMenu"></div>
			<div id="subMenu"></div>
		</td>
		<td valign="top">
			<div id="tabMenu" class="submenuBlock">
				<table border="0" cellspacing="0" cellpadding="0">
					<tbody>
					<tr>
						<td>
							<a href="cloud_main.asp"><div class="tab"><span>AiCloud</span></div></a>
						</td>
						<td>
							<div class="tabclick"><span>Smart Sync</span></div>
						</td>
					</tr>
					</tbody>
				</table>
			</div>

		<!--==============Beginning of hint content=============-->
			<table width="98%" border="0" align="left" cellpadding="0" cellspacing="0">
			  <tr>
					<td align="left" valign="top">
					  <table width="100%" border="0" cellpadding="5" cellspacing="0" class="FormTitle" id="FormTitle">
						<tbody>
						<tr>
						  <td bgcolor="#4D595D" valign="top">

						<div>&nbsp;</div>
						<div class="formfonttitle">Smart Sync</div>
						<div style="margin-left:5px;margin-top:10px;margin-bottom:10px"><img src="/images/New_ui/export/line_export.png"></div>
				
						<div>
							<table width="700px" style="margin-left:25px;">
								<tr>
									<td>
										<img id="guest_image" src="/images/cloudsync/003.png" style="margin-top:10px;margin-left:-20px">
										<div align="center" class="left" style="margin-top:25px;margin-left:43px;width:94px; float:left; cursor:pointer;" id="radio_smartSync_enable"></div>
										<div class="iphone_switch_container" style="height:32px; width:74px; position: relative; overflow: hidden">
										<script type="text/javascript">
											$j('#radio_smartSync_enable').iphoneSwitch('<% nvram_get("enable_cloudsync"); %>',
												function() {
													document.enableform.enable_cloudsync.value = 1;
													showLoading();	
													document.enableform.submit();
												},
												function() {
													document.enableform.enable_cloudsync.value = 0;
													showLoading();	
													document.enableform.submit();	
												},
												{
													switch_on_container_path: '/switcherplugin/iphone_switch_container_off.png'
												}
											);
										</script>			
										</div>

									</td>
									<td>&nbsp;&nbsp;</td>
									<td>
										<div style="padding:10px;width:95%;font-style:italic;font-size:14px;word-break:break-all;">
												<#smart_sync1#><br />
												<#smart_sync2#>											
										</div>
									</td>
								</tr>
							</table>
						</div>

   					<table width="99%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table" id="cloudlistTable" style="margin-top:30px;">
	  					<thead>
	   					<tr>
	   						<td colspan="6" id="cloud_synclist">Cloud List</td>
	   					</tr>
	  					</thead>		  

    					<tr>
      					<th width="10%"><!--a class="hintstyle" href="javascript:void(0);" onClick="openHint(18,2);"-->Provider<!--/a--></th>
    						<th width="25%"><#PPPConnection_UserName_itemname#></a></th>
      					<th width="10%">Rule</a></th>
      					<th width="30%">Folder</th>
      					<th width="15%">Status</th>
      					<th width="10%"><#CTL_del#></th>
    					</tr>

						</table>
	
						<div id="cloud_synclist_Block"></div>

					  <table width="99%" border="1" align="center" cellpadding="4" cellspacing="0" bordercolor="#6b8fa3" class="FormTable" id="cloudAddTable" style="margin-top:10px;display:none;">
	  					<thead>
	   					<tr>
	   						<td colspan="6" id="cloud_synclist">Cloud List</td>
	   					</tr>
	  					</thead>		  

							<tr>
							<th width="30%" style="height:40px;font-family: Calibri;font-weight: bolder;">
								Provider
							</th>
							<td>
								<div><img style="margin-top: -2px;" src="/images/cloudsync/ASUS-WebStorage.png"></div>
								<div style="font-size:18px;font-weight: bolder;margin-left: 45px;margin-top: -27px;font-family: Calibri;">ASUS WebStorage</div>
							</td>
							</tr>
				            
						  <tr>
							<th width="30%" style="font-family: Calibri;font-weight: bolder;">
								<#AiDisk_Account#>
							</th>			
							<td>
							  <input type="text" maxlength="32" class="input_32_table" style="height: 25px;" id="cloud_username" name="cloud_username" value="" onKeyPress="">
							</td>
						  </tr>	

						  <tr>
							<th width="30%" style="font-family: Calibri;font-weight: bolder;">
								<#PPPConnection_Password_itemname#>
							</th>			
							<td>
								<input id="cloud_password" name="cloud_password" type="password" autocapitalization="off" onBlur="switchType(false);" onFocus="switchType(true);switchType_IE(this);" maxlength="32" class="input_32_table" style="height: 25px;" value="">
							  <input id="cloud_password_text" name="cloud_password_text" type="text" autocapitalization="off" onBlur="switchType_IE(this);" maxlength="32" class="input_32_table" style="height:25px; display:none;" value="">
							</td>
						  </tr>						  				
					  				
						  <tr>
							<th width="30%" style="font-family: Calibri;font-weight: bolder;">
								Folder
							</th>
							<td>
			          <input type="text" id="PATH" class="input_32_table" style="height: 25px;" name="cloud_dir" value="" onclick=""/>
		  					<input name="button" type="button" class="button_gen_short" onclick="get_disk_tree();" value="Browser"/>
								<div id="noUSB" style="color:#FC0;display:none;margin-left: 3px;"><#no_usb_found#></div>
							</td>
						  </tr>

						  <tr>
							<th width="30%" style="font-family: Calibri;font-weight: bolder;">
								Rule
							</th>
							<td>
								<select name="cloud_rule" class="input_option">
									<option value="0">Sync</option>
									<option value="1">Download to USB Disk</option>
									<option value="2">Upload to Cloud</option>
								</select>			
							</td>
						  </tr>
						</table>	
					
   					<table width="98%" border="1" align="center" cellpadding="4" cellspacing="0" class="FormTable_table" id="cloudmessageTable" style="margin-top:7px;display:none;">
	  					<thead>
	   					<tr>
	   						<td colspan="6" id="cloud_synclist">Cloud Message</td>
	   					</tr>
	  					</thead>		  
							<tr>
								<td>
									<textarea style="width:99%; border:0px; font-family:'Courier New', Courier, mono; font-size:13px;background:#475A5F;color:#FFFFFF;" cols="63" rows="25" readonly="readonly" wrap=off ></textarea>
								</td>
							</tr>
						</table>

	  				<div class="apply_gen" id="creatBtn" style="margin-top:30px;display:none;">
							<input name="applybutton" id="applybutton" type="button" class="button_gen_long" onclick="isEdit=1;showAddTable();" value="<#AddAccountTitle#>" style="word-wrap:break-word;word-break:normal;">
							<img id="update_scan" style="display:none;" src="images/InternetScan.gif" />
	  				</div>

	  				<div class="apply_gen" style="margin-top:30px;display:none;" id="applyBtn">
	  					<input name="button" type="button" class="button_gen" onclick="isEdit=0;showAddTable();" value="<#CTL_Cancel#>"/>
	  					<input name="button" type="button" class="button_gen" onclick="applyRule()" value="<#CTL_apply#>"/>
	  				</div>

					  </td>
					</tr>				
					</tbody>	
				  </table>		
	
					</td>
				</tr>
			</table>
		</td>
		<td width="20"></td>
	</tr>
</table>
<div id="footer"></div>
</form>
<form method="post" name="enableform" action="/start_apply.htm" target="hidden_frame">
<input type="hidden" name="preferred_lang" id="preferred_lang" value="<% nvram_get("preferred_lang"); %>">
<input type="hidden" name="firmver" value="<% nvram_get("firmver"); %>">
<input type="hidden" name="current_page" value="cloud_sync.asp">
<input type="hidden" name="next_page" value="cloud_sync.asp">
<input type="hidden" name="next_host" value="">
<input type="hidden" name="modified" value="0">
<input type="hidden" name="action_mode" value="apply">
<input type="hidden" name="action_script" value="restart_cloudsync">
<input type="hidden" name="action_wait" value="2">
<input type="hidden" name="enable_cloudsync" value="<% nvram_get("enable_cloudsync"); %>">
</form>
</body>
</html>

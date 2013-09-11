function wl_chanspec_list_change(){
	var phytype = "n";
	var band = document.form.wl_unit.value;
	var bw_cap = document.form.wl_bw.value;
	var chanspecs = new Array(0);
	var cur = 0;
	var sel = 0;

	if(country == ""){
		country = prompt("The Country Code is not exist! Please enter Country Code.", "");
	}

	/* Save current chanspec */
	cur = '<% nvram_get("wl_chanspec"); %>';

	if (phytype == "a") {	// a mode
		chanspecs = new Array(0); 
	}
	else if (phytype == "n") { // n mode
		if (band == "1") { // - 5 GHz
			if (bw_cap == "1") { // -- 20 MHz
				if (country == "Q2")
					chanspecs = new Array(0, "36", "40", "44", "48", "149", "153", "157", "161", "165");
				else if (country == "EU")
					chanspecs = new Array(0, "36", "40", "44", "48");
				else if (country == "TW")
					chanspecs = new Array(0, "56", "60", "64", "149", "153", "157", "161", "165");
				else if (country == "CN")
					chanspecs = new Array(0, "149", "153", "157", "161", "165");
				else if (country == "XX")
					chanspecs = new Array(0, "34", "36", "38", "40", "42", "44", "46", "48", "52", "56", "60", "64", "100", "104", "108", "112", "116", "120", "124", "128", "132", "136", "140", "144", "149", "153", "157", "161", "165");
				else // US
					chanspecs = new Array(0, "36", "40", "44", "48", "149", "153", "157", "161", "165");
			} 
			else if (bw_cap == "2") { // -- 40 MHz
				if (country == "Q2")
					chanspecs = new Array(0, "36l", "40u", "44l", "48u", "149l", "153u", "157l", "161u");
				else if (country == "EU")
					chanspecs = new Array(0, "36l", "40u", "44l", "48u");
				else if (country == "TW")
					chanspecs = new Array(0, "60l", "64u", "149l", "153u", "157l", "161u");
				else if (country == "CN")
					chanspecs = new Array(0, "149l", "153u", "157l", "161u");
				else if (country == "XX")
					chanspecs = new Array(0, "36l", "40u", "44l", "48u", "52l", "56u", "60l", "64u", "100l", "104u", "108l", "112u", "116l", "120u", "124l", "128u", "132l", "136u", "140l", "144u", "149l", "153u", "157l", "161u");
				else // US
					chanspecs = new Array(0, "36l", "40u", "44l", "48u", "149l", "153u", "157l", "161u");
			} 
			else if (bw_cap == "3" || bw_cap == "0") { // -- 80 MHz
				if (country == "Q2")
					chanspecs = new Array(0, "36/80", "40/80", "44/80", "48/80", "149/80", "153/80", "157/80", "161/80");
				else if (country == "EU")
					chanspecs = new Array(0, "36/80", "40/80", "44/80", "48/80");
				else if (country == "TW")
					chanspecs = new Array(0, "149/80", "153/80", "157/80", "161/80");
				else if (country == "CN")
					chanspecs = new Array(0, "149/80", "153/80", "157/80", "161/80");
				else if (country == "XX")
					chanspecs = new Array(0, "36/80", "40/80", "44/80", "48/80", "52/80", "56/80", "60/80", "64/80", "100/80", "104/80", "108/80", "112/80", "116/80", "120/80", "124/80", "128/80", "132/80", "136/80", "140/80", "144/80", "149/80", "153/80", "157/80", "161/80");
				else // US
					chanspecs = new Array(0, "36/80", "40/80", "44/80", "48/80", "149/80", "153/80", "157/80", "161/80");
			} 
			else { // auto
				chanspecs = [0];
			}
		} 
		else if (band == "0") { // - 2.4 GHz
			if (bw_cap == "1") { // -- 20 MHz
				if (country == "Q2")
					chanspecs = new Array(0, "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11");
				else if (country == "EU")
					chanspecs = new Array(0, "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13");
				else if (country == "TW")
					chanspecs = new Array(0, "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11");
				else if (country == "CN")
					chanspecs = new Array(0, "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13");
				else if (country == "XX")
					chanspecs = new Array(0, "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11", "12", "13", "14");
				else // US
					chanspecs = new Array(0, "1", "2", "3", "4", "5", "6", "7", "8", "9", "10", "11");
			} 
			else if (bw_cap == "2" || bw_cap == "0") { // -- 40 MHz
				if (country == "Q2")
					chanspecs = new Array(0, "1l", "2l", "3l", "4l", "5l", "5u", "6l", "6u", "7l", "7u", "8u", "9u", "10u", "11u");
				else if (country == "EU")
					chanspecs = new Array(0, "1l", "2l", "3l", "4l", "5l", "5u", "6l", "6u", "7l", "7u", "8l", "8u", "9l", "9u", "10u", "11u", "12u", "13u");
				else if (country == "TW")
					chanspecs = new Array(0, "1l", "2l", "3l", "4l", "5l", "5u", "6l", "6u", "7l", "7u", "8u", "9u", "10u", "11u");
				else if (country == "CN")
					chanspecs = new Array(0, "1l", "2l", "3l", "4l", "5l", "5u", "6l", "6u", "7l", "7u", "8l", "8u", "9l", "9u", "10u", "11u", "12u", "13u");
				else if (country == "XX")
					chanspecs = new Array(0, "1l", "2l", "3l", "4l", "5l", "5u", "6l", "6u", "7l", "7u", "8l", "8u", "9l", "9u", "10u", "11u", "12u", "13u");
				else // US
					chanspecs = new Array(0, "1l", "2l", "3l", "4l", "5l", "5u", "6l", "6u", "7l", "7u", "8u", "9u", "10u", "11u");
			}
			else { // auto
				chanspecs = [0];
			}
		}
	} 
	else { // b/g mode 
		chanspecs = new Array(0);
	}

	/* displaying chanspec even if the BW is auto.
	if(chanspecs[0] == 0 && chanspecs.length == 1)
		document.form.wl_chanspec.parentNode.parentNode.style.display = "none";
	else
		document.form.wl_chanspec.parentNode.parentNode.style.display = "";
	*/

	/* Reconstruct channel array from new chanspecs */
	document.form.wl_chanspec.length = chanspecs.length;
	for (var i in chanspecs) {
		if (chanspecs[i] == 0){
			document.form.wl_chanspec[i] = new Option("<#Auto#>", chanspecs[i]);
		}
		else{
			if(band == "1")
				document.form.wl_chanspec[i] = new Option(chanspecs[i].toString().replace("/80", "").replace("u", "").replace("l", ""), chanspecs[i]);
			else
				document.form.wl_chanspec[i] = new Option(chanspecs[i].toString(), chanspecs[i]);
		}

		document.form.wl_chanspec[i].value = chanspecs[i];
		if (chanspecs[i] == cur) {
			document.form.wl_chanspec[i].selected = true;
			sel = 1;
		}
	}

	if (sel == 0 && document.form.wl_chanspec.length > 0)
		document.form.wl_chanspec[0].selected = true;
}

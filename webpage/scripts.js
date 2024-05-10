function get_data(val) {
	jQuery.ajax({
		url: 'json_input',
		type: 'GET',
		timeout: 5000,
		success: function(data) {
			console.log("Incommming data: ");
			console.log(data);
			var obj = JSON.parse(data);
			console.log(obj);

			if (!document.querySelector('#light-icon img')) {
				var img = document.createElement('img');
				img.src = (obj.led == "true") ? 'light-on-icon.svg' : 'light-off-icon.svg';
				document.getElementById('light-icon').appendChild(img);
			}

			if (val == "config") {
				$("#fingerprint").text(obj.fingerprint);
				$("#refreshInterval").text(obj.refreshInterval);
				document.getElementById('tokenid').value = obj.token;
				document.getElementById('refreshid').value = obj.refreshInterval;
				document.getElementById('photo_qualityid').value = obj.photoquality;
				document.getElementById('framesizeid').value = obj.framesize;
				document.getElementById('brightnessid').value = obj.brightness;
				document.getElementById('contrastid').value = obj.contrast;
				document.getElementById('saturationid').value = obj.saturation;
				document.getElementById('hmirrorid').checked = obj.hmirror;
				document.getElementById('vflipid').checked = obj.vflip;
				document.getElementById('lencid').checked = obj.lensc;
				document.getElementById('exposure_ctrlid').checked = obj.exposure_ctrl;
				document.getElementById('awbid').checked = obj.awb;
				document.getElementById('awb_gainid').checked = obj.awb_gain;
				document.getElementById('wb_modeid').value = obj.wb_mode;
				document.getElementById('ledid').checked = obj.led;
				document.getElementById('flashid').checked = obj.flash;
				document.getElementById('flash_timeid').value = obj.flash_time;
				document.getElementById('bpcid').checked = obj.bpc;
				document.getElementById('wpcid').checked = obj.wpc;
				document.getElementById('raw_gamaid').checked = obj.raw_gama;
				document.getElementById('aec2id').checked = obj.aec2;
				document.getElementById('ae_levelid').value = obj.ae_level;
				document.getElementById('aec_valueid').value = obj.aec_value;
				document.getElementById('gain_ctrlid').checked = obj.gain_ctrl;
				document.getElementById('agc_gainid').value = obj.agc_gain;
				document.getElementById("flash_time_value").innerText = obj.flash_time;
				document.getElementById("aec_value_value").innerText = obj.aec_value;
				$("#status_hmirror").text((obj.hmirror == "true") ? "On" : "Off");
				$("#status_vflip").text((obj.vflip == "true") ? "On" : "Off");
				$("#status_lensc").text((obj.lensc == "true") ? "On" : "Off");
				$("#status_exposure_ctrl").text((obj.exposure_ctrl == "true") ? "On" : "Off");
				$("#status_awb").text((obj.awb == "true") ? "On" : "Off");
				$("#status_awb_gain").text((obj.awb_gain == "true") ? "On" : "Off");
				$("#status_led").text((obj.led == "true") ? "On" : "Off");
				$("#status_flash").text((obj.flash == "true") ? "On" : "Off");
				$("#status_bpc").text((obj.bpc == "true") ? "On" : "Off");
				$("#status_wpc").text((obj.wpc == "true") ? "On" : "Off");
				$("#status_raw_gama").text((obj.raw_gama == "true") ? "On" : "Off");
				$("#status_aec2").text((obj.aec2 == "true") ? "On" : "Off");
				$("#status_gain_ctrl").text((obj.gain_ctrl == "true") ? "On" : "Off");
				sliderCheck();
			}

			if (val == "auth") {
				document.getElementById('authid').checked = obj.auth;
				$("#status_auth").text((obj.auth == "true") ? "On" : "Off");
				document.getElementById('auth_username').value = obj.auth_username;
			}

			if (val == "wifi") {
				document.getElementById('serviceapid').checked = obj.serviceap;
				$("#status_serviceap").text((obj.serviceap == "true") ? "On" : "Off");
				$("#ssid").text(obj.ssid);
				$("#rssi").text(obj.rssi);
				$("#rssi_percentage").text(obj.rssi_percentage);
				$("#ip").text(obj.ip);
				$("#mdns").text(obj.mdns);
				$("#wifi_network_status").text(obj.wifi_network_status);
				document.getElementById('ipcfgid').value = obj.ip_cfg;
				document.getElementById('net_ip_id').value = obj.net_ip;
				document.getElementById('net_mask_id').value = obj.net_mask;
				document.getElementById('net_gw_id').value = obj.net_gw;
				document.getElementById('net_dns_id').value = obj.net_dns;

				if (!document.querySelector('#main-wifi-signal wifi_img')) {
					var wifi_img = document.createElement('wifi_img');
					wifi_img.width = 19;
					wifi_img.height = 12;
					wifi_img.src = getIconPath(obj.rssi);
					document.getElementById('main-wifi-signal').appendChild(wifi_img);
				}
			}

			if (val == "system") {
				$("#uptime").text(obj.uptime);
				$("#sw_ver").text(obj.sw_ver);
				$("#sw_build").text(obj.sw_build);
				$("#last_upload_status").text(obj.last_upload_status);
				$("#wifi_mode").text(obj.wifi_mode);
				$("#sw_new_ver").text(obj.sw_new_ver);
				$("#service_ap_ssid").text(obj.service_ap_ssid);
				document.getElementById('mdnsid').value = obj.mdns;
        		document.getElementById('loglevelid').value = obj.log_level;
			}
		},
		error: function(html) {
			console.log("json Timeout or error");
			//alert("jquery timeout or comunication error");
		}
	});
}

function sliderCheck() {
	var ranges = document.querySelectorAll(".slider");
	ranges.forEach(function(range) {
		var percent = (range.value - range.min) / (range.max - range.min) * 100;
		var gradient = "linear-gradient(to right, #FA6831 " + percent + "%, #d3d3d3 " + percent + "%)";
		range.style.background = gradient;
		range.oninput = function() {
			var percent = (this.value - this.min) / (this.max - this.min) * 100;
			var gradient = "linear-gradient(to right, #FA6831 " + percent + "%, #d3d3d3 " + percent + "%)";
			this.style.background = gradient;
		}
	});
}

function getIconPath(rssi) {
	let path;
	if (rssi == 0) {
		path = 'wifi-icon-0.svg';
	} else if (rssi <= -70) {
		path = 'wifi-icon-1.svg';
	} else if (rssi > -70 && rssi <= -60) {
		path = 'wifi-icon-2.svg';
	} else if (rssi > -60 && rssi <= -50) {
		path = 'wifi-icon-3.svg';
	} else {
		path = 'wifi-icon-4.svg';
	}
	return path;
}

var OpenImageclickCount = 0;

function openImage() {
	var img = document.getElementById("photo");
	if (OpenImageclickCount % 2 == 0) {
		img.style.position = "fixed";
		img.style.top = "5%";
		img.style.left = "5%";
		img.style.width = "auto";
		img.style.height = "auto";
		img.style.maxWidth = "100%";
		img.style.maxHeight = "90%";
		img.style.zIndex = "9999";
	} else {
		img.style.position = "";
		img.style.top = "";
		img.style.left = "";
		img.style.width = "";
		img.style.height = "";
		img.style.zIndex = "";
	}
	OpenImageclickCount++;
}

function actionButton(url, reload, msg) {
    var xhr = new XMLHttpRequest();
    
	if (msg != '') {
		alert(msg);
	}

    xhr.onreadystatechange = function() {
        if (xhr.readyState == 4 && xhr.status == 200) {
            if (reload == true) {
                setTimeout(function() {
                    location.reload();
                }, 200);
            }
        }
    };

    xhr.open('GET', url, true);
    xhr.send();
}

function setActive(link) {
	var links = document.querySelectorAll('#links li a');
	links.forEach(function(item) {
		item.classList.remove('active');
	});
	link.classList.add('active');
}

var links = document.querySelectorAll('#links li a');
links.forEach(function(link) {
	link.addEventListener('click', function() {
		setActive(link);
	});
});

function addClickListener(id) {
    var link = document.getElementById(id);
    if (!link.hasOwnProperty('clickListener')) {
        link.addEventListener('click', function(event) {
            event.preventDefault();
            window.open(link.href, '_blank');
        });
        link.clickListener = true;
    }
}

/* wifi page */
function setWifi(val_ssid, val_pass) {
	var xmlHttp = new XMLHttpRequest();
	xmlHttp.open("GET", "wifi_cfg?wifi_ssid=" +  encodeURIComponent(val_ssid) + "&wifi_pass=" + encodeURIComponent(val_pass), false);
	xmlHttp.send(null);
	alert(xmlHttp.responseText);
	get_data("wifi");
}

function setWifiNet(val_ip, val_mask, val_gw, val_dns) {	
	var xmlHttp = new XMLHttpRequest();
	xmlHttp.open("GET", "wifi_net_cfg?ip=" +  encodeURIComponent(val_ip) + "&mask=" + encodeURIComponent(val_mask) + "&gw=" + encodeURIComponent(val_gw) + "&dns=" + encodeURIComponent(val_dns), false);
	xmlHttp.send(null);
	alert(xmlHttp.responseText);
	get_data("wifi");
}

function scanWifi() {
	var xmlHttp = new XMLHttpRequest();
	xmlHttp.open("GET", "wifi_scan?", false);
	xmlHttp.send(null);
	alert(xmlHttp.responseText);
	get_data("wifi");
	setTimeout(function() {
		GetDataAndPrintTableWiFi();
	}, 8000);
}

function GetDataAndPrintTableWiFi() {
	$("#wifi_ntw").find("tr:gt(0)").remove();
	$.ajax({
		url: 'json_wifi',
		type: 'GET',
		timeout: 15000,
		dataType: 'json',
		data: {},

		success: function(data) {
			for (var i = 0; i < data.length; i++) {
				const IconName = "wifi-icon-" + i;
				var row = $('<tr><td>' + data[i].ssid + '</td><td><div id="' + IconName + '" style="display: flex; align-items: center;"></div></td><td>' + data[i].channel + '</td><td>' + data[i].encryption + '</td></tr>');
				$('#wifi_ntw').append(row);

				if (!document.querySelector('#' + IconName + ' img')) {
					var img = document.createElement('img');
					img.src = getIconPath(data[i].rssi);
					document.getElementById(IconName).prepend(img);
					document.getElementById(IconName).append(data[i].rssi_percentage);
					document.getElementById(IconName).append("% / ");
					document.getElementById(IconName).append(data[i].rssi);
					document.getElementById(IconName).append("dBm");
				}
			}
		},
		error: function(jqXHR, textStatus, errorThrown) {
			console.log('Error:' + textStatus + '-' + errorThrown);
		}
	});
}

/* auth page */
function setAuth(val_name, val_pass) {
	var xmlHttp = new XMLHttpRequest();
	xmlHttp.open("GET", "basicauth_cfg?" + "auth_username=" + encodeURIComponent(val_name) + "&auth_password=" + encodeURIComponent(val_pass), false);
	xmlHttp.send(null);
	alert(xmlHttp.responseText);
	get_data("auth");
}

function changeValue(val, url, reload, msg) {
	var xmlHttp = new XMLHttpRequest();
	xmlHttp.open("GET", url + val, false);
	xmlHttp.send(null);
	if ((url == "set_int?refresh=") || (url == "set_token?token=") || (url == "set_mdns?mdns=") || (url == "set_int?ipcfg=")) {
		alert(xmlHttp.responseText);
	}
	if (url == "set_flash_time?flash_time=") {
		document.getElementById("flash_time_value").innerText = val;
	}
	get_data(reload);
}

function togglePasswordVisibility() {
	const passwordInput = document.getElementById("auth_password");
	const eyeIcon = document.getElementById("eye-icon");
	if (passwordInput.getAttribute("type") === "password") {
		passwordInput.setAttribute("type", "text");
		passwordInput.classList.add("reveal");
		eyeIcon.src = "eye-slash.svg";
		eyeIcon.alt = "Hide Password";
	} else {
		passwordInput.setAttribute("type", "password");
		passwordInput.classList.remove("reveal");
		eyeIcon.src = "eye.svg";
		eyeIcon.alt = "Show Password";
	}
}

/* system page */
if (typeof uploadingFirmware === 'undefined') {
	var uploadingFirmware = false;
}

if (typeof FileSize === 'undefined') {
	var FileSize = 0;
}

function uploadFile() {
	alert("Started updating...");
	const firmwareInput = document.getElementById('firmwareInput');
	const statusDiv = document.getElementById('status');
	const file = firmwareInput.files[0];
	FileSize = file.size;
	SetFirmwareSize(file.size);

	if (file) {
		statusDiv.innerText = 'Updating...';
		uploadingFirmware = true;
		const formData = new FormData();
		formData.append('firmware', file);

		fetch('/upload', {
				method: 'POST',
				body: formData,
			})
			.then((response) => {
				if (response.ok) {
					response.text().then((data) => {
						const jsonData = JSON.parse(data);
						updateProgress();
						uploadingFirmware = false;
						if (jsonData.errorMessage) {
							alert(`Error message: ${jsonData.errorMessage}`);
						}
					});
				} else {
					uploadingFirmware = false;
					response.text().then((errorMessage) => {
						alert(`Error message: ${errorMessage}`);
					});
				}
			})
			.catch((error) => {
				console.error('Error:', error);
				uploadingFirmware = false;
			});
	} else {
		statusDiv.innerText = 'No file selected';
	}
}

function SetFirmwareSize(val) {
	var xmlHttp = new XMLHttpRequest();
	xmlHttp.open("GET", "set_firmware_size?size=" + val, false);
	xmlHttp.send(null);
}

function updateProgress() {
	if (!uploadingFirmware) {
		return;
	}

	fetch('/UpdateProcessing', {
			method: 'GET',
		})
		.then((response) => {
			if (response.ok) {
				return response.json();
			} else {
				throw new Error('Failed to fetch progress');
			}
		})
		.then((data) => {
			const statusDiv = document.getElementById('status');
			var progressBar = document.getElementById("myProgressBar");
			progressBar.style.width = data.processed_percent + "%";
			progressBar.innerHTML = data.processed_percent + "%";

			statusDiv.innerText = data.message;
			uploadingFirmware = data.updating;
			if (data.updating == false  && !updateCompleted) {
				alert('Operation done. Please reboot MCU.');
				uploadingFirmware = false;
				updateCompleted = true;
				clearInterval(updateInterval);
			}
		})
		.catch((error) => {
			console.error('Error:', error);
			var progressBar = document.getElementById("myProgressBar");
			progressBar.innerHTML = "Error";
			clearInterval(updateInterval);
		});
}

function checkUpdate() {
	var xmlHttp = new XMLHttpRequest();
	alert("Connecting to server... Please wait several second");
	xmlHttp.open("GET", "/check_web_ota_update", false);
	xmlHttp.send(null);
	alert(xmlHttp.responseText);
	get_data("system");
}

function updateWeb() {
	alert("Started updating from cloud.");
	var xmlHttp = new XMLHttpRequest();
	xmlHttp.open("GET", "web_ota_update?update=true", false);
	xmlHttp.send(null);
	uploadingFirmware = true;
}

function validatePasswords() {
    var password = document.getElementById("auth_password").value;
    var confirmPassword = document.getElementById("auth_password_confirm").value;
    var saveButton = document.querySelector(".btn_save_a");
    var passwordStatus = document.getElementById("pass_match");

    if (password === confirmPassword) {
        passwordStatus.innerHTML = "✔️";
        saveButton.disabled = false;
    } else {
        passwordStatus.innerHTML = "❌";
        saveButton.disabled = true;
    }
}

if (document.getElementById("auth_password")) {
	document.getElementById("auth_password").addEventListener("input", validatePasswords);
}

if (document.getElementById("auth_password_confirm")) {
	document.getElementById("auth_password_confirm").addEventListener("input", validatePasswords);
}

if ((document.getElementById("auth_password")) && (document.getElementById("auth_password_confirm"))) {
	validatePasswords();
}

function setupCollapsibleButtons() {
    $(".btn_collapsible").click(function(){
        $(this).toggleClass("active");
        var content = $(this).parent().next();
        if (content.css("display") === "block") {
            content.css("display", "none");
        } else {
            content.css("display", "block");
        }
    });
}

function setupCollapsibleButtonsWiFi() {
    $(".btn_collapsible_wifi").click(function(){
        $(this).toggleClass("active");
        var content_wifi = $(this).parent().next();
        if (content_wifi.css("display") === "block") {
            content_wifi.css("display", "none");
        } else {
            content_wifi.css("display", "block");
        }
    });
}

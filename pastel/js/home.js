var minSocialPanelWidth = 100;

var homeMainInner;
var homeSocial;

var lastMListSubmitButton;

function onLoad() {
	homeMainInner = document.getElementById("home-main-inner");
	homeSocial = document.getElementById("home-social");

	onResize();
	window.addEventListener("resize", function () {
		onResize();
	});

	parent.location.hash = "home";

	let reloadAudio = function() {
		let index = Math.floor(Math.random() * 759 + 1);
		let songStr = "/prototype/" + ("0000" + index.toString()).slice(-5) + ".mp3";
		document.getElementById("home-projects-audio").src = songStr;
		document.getElementById("home-projects-audio").load();
		document.getElementById("home-projects-audio").play();
	};
	document.getElementById("home-projects-random").onclick = reloadAudio;
	document.getElementById("home-projects-audio").volume = 0.5;
	document.getElementById("home-projects-audio").onended = reloadAudio;

	//get client IP
	let ipReq = new XMLHttpRequest();
	ipReq.open("GET", "/scripts/print-ip.exe");
	ipReq.onreadystatechange = function () {
		if (ipReq.readyState === 4) {
			document.getElementById("home-gilgamesh-ip").textContent += ipReq.response;
		}
	}
	ipReq.send();

	//get server version
	let verReq = new XMLHttpRequest();
	verReq.open("GET", "/scripts/print-ver.exe");
	verReq.onreadystatechange = function () {
		if (verReq.readyState === 4) {
			document.getElementById("home-gilgamesh-version").textContent += verReq.response;
		}
	}
	verReq.send();

	//get last modified time
	let lastMod = new XMLHttpRequest();
	lastMod.open("GET", "/scripts/print-last-mod.exe");
	lastMod.onreadystatechange = function () {
		if (lastMod.readyState === 4) {
			document.getElementById("home-gilgamesh-lastmod").textContent += lastMod.response;
		}
	}
	lastMod.send();

	homeMainInner.parentNode.style.opacity = 1;
}
onLoad();

function onResize() {
	document.getElementsByTagName("html")[0].style.setProperty("--social-panel-cols", Math.min(4, Math.ceil(homeSocial.clientWidth / minSocialPanelWidth)));
}

function validateEmail(email) {
    let re = /^(([^<>()\[\]\\.,;:\s@"]+(\.[^<>()\[\]\\.,;:\s@"]+)*)|(".+"))@((\[[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\.[0-9]{1,3}\])|(([a-zA-Z\-0-9]+\.)+[a-zA-Z]{2,}))$/;
    return re.test(String(email).toLowerCase());
}

function mListFormSubmit(form) {
	let action = lastMListSubmitButton;
	let email = document.getElementsByName("home-mlist-form-email")[0].value;
	let responseElem = document.getElementById("home-mlist-form-response");

	if (!validateEmail(email)) {
		responseElem.textContent = "Not a valid email.";
		return false;
	}
	responseElem.textContent = "Waiting for server response...";

	let request = new XMLHttpRequest();
	request.open("GET", "/scripts/update-mlist-subs.exe?action=" + encodeURI(action) + "&email=" + encodeURI(email));
	request.onreadystatechange = function () {
		if (request.readyState === 4) {
			responseElem.textContent = "Server response: " + request.response + ".";
		}
	}
	request.send();

	return false;
}

function emailFormSubmit(form) {
	let from = document.getElementById("home-email-from").value;
	let fromName = document.getElementById("home-email-from-name").value;
	let to = document.getElementById("home-email-to").value;
	let toName = document.getElementById("home-email-to-name").value;
	let subject = document.getElementById("home-email-subject").value;
	let data = document.getElementById("home-email-data").value;

	let responseElem = document.getElementById("home-email-form-response");

	if (!validateEmail(to)) {
		responseElem.textContent = "Not a valid `to` email.";
		return false;
	}
	responseElem.textContent = "Waiting for server response...";

	data = btoa(unescape(encodeURIComponent(data)));
	data = "MIME-Version: 1.0\r\n" + 
		"From: " + fromName + " <" + from + ">\r\n" + 
		"To: " + toName + " <" + to + ">\r\n" + 
		"Subject: " + subject + "\r\n" + 
		"Content-Type: text/plain\r\n" + 
		"Content-Transfer-Encoding: base64\r\n" + 
		"Date: " + new Date().toUTCString() + "\r\n" + 
		"\r\n" + 
		data;

	let request = new XMLHttpRequest();
	request.open("GET", "/scripts/access-smtp.exe?from=" + encodeURI("server@emilia-tan.com") + "&to=" + encodeURI(to) + "&data=" + encodeURI(data));
	request.onreadystatechange = function() {
		if (request.readyState === 4) {
			if (request.response.split(" ").pop().trim() == "OK") {
				responseElem.textContent = "Email sent!";
			} else {
				responseElem.textContent = "There was an error. Emilia probably wasn't able to connect to the remote SMTP server...";
			}
		}
	}
	request.send();

	return false;
}

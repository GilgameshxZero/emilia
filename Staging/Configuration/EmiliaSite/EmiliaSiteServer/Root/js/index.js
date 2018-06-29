let navSelected = "nav-selected";
let defaultNav = "home";

var navBar;
var mainDiv;

var curNavSelect = null;

window.onload = function() {
	navBar = document.getElementById("navigator");
	mainDiv = document.getElementById("main");

	var fragment = window.location.hash.substr(1);
	fragment = fragment.split("&", 2)[0];

	setNavBar(fragment == "" ? defaultNav : fragment);

	for (var a = 0;a < navBar.children.length;a++) {
		navBar.children[a].addEventListener("click", onNavClick, false);
	}
}

function fetchText(url) {
    return fetch(url).then(function(response) {
		return response.text();
	});
}

function loadJS(url, implementationCode, location){
    var scriptTag = document.createElement('script');
    scriptTag.src = url;

    scriptTag.onload = implementationCode;
    scriptTag.onreadystatechange = implementationCode;

    location.appendChild(scriptTag);
}

function setNavBar(id) {
	//update main content
	fetchText(id + ".html").then(function(html) {
		mainDiv.innerHTML = html;
		
		loadJS("js/" + id + ".js", function() {
			//do nothing
		}, document.body);
    }).catch(function (error) {
        console.warn(error);
    });

	//change navbar style
	if (curNavSelect != null)
		document.getElementById(curNavSelect).classList.remove(navSelected);
	document.getElementById(id).classList.add(navSelected);
	curNavSelect = id;

	//TODO: modify URL
}

function onNavClick(event) {
	if (this.classList.contains(navSelected))
		return;

	setNavBar(this.id);
}

function moveChildren(src, dst) {
	while (src.childNodes.length > 0) {
		dst.appendChild(src.childNodes[0]);
	}
}